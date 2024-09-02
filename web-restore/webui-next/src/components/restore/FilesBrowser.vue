<script setup lang="ts">
import { useWizardStore } from 'src/stores/wizardStore'
import { ref, watch } from 'vue'
import { File, FileType } from 'src/generated/restore'
import {
  matQuestionMark,
  matInsertDriveFile,
  matFolder,
} from '@quasar/extras/material-icons'
import { storeToRefs } from 'pinia'

const wizardStore = useWizardStore()

const { currentDirectory, files } = storeToRefs(wizardStore)

const getIcon = (type: FileType) => {
  switch (type) {
    case FileType.DIRECTORY:
      return matFolder
    case FileType.FILE:
      return matInsertDriveFile
    case FileType.DIRECTORY_NOT_BACKED_UP:
      return matFolder
    default:
      return matQuestionMark
  }
}

const changeDirectory = (name: File, event: Event) => {
  event.preventDefault()
  console.debug('changeDirectory:', name)
  wizardStore.changeDirectory(name)
}

const isDirectory = (row: File) => {
  return row.type !== FileType.FILE
}

const selected = ref<File>()

watch(selected, (file: File | undefined) => {
  console.debug('selected:', file)
  if (!file) {
    console.info('file is undefined')
    return
  }
  wizardStore.changeDirectory(file)
})

const updateMarkedStatus = async (value: boolean, file: File) => {
  console.log('updateMarking:', value, file)
  await wizardStore.updateMarkedStatus(file)
}
</script>

<template>
  <q-breadcrumbs gutter="sm">
    <template v-for="(breadcrumb, index) in currentDirectory" :key="index">
      <template v-if="breadcrumb.id?.value != 0n">
        <q-breadcrumbs-el
          to="#"
          :label="breadcrumb.name"
          @click="(event) => changeDirectory(breadcrumb, event)"
        />
      </template>
      <template v-else>
        <q-breadcrumbs-el
          to="#"
          :label="breadcrumb.name"
          @click="(event) => changeDirectory(breadcrumb, event)"
          icon="home"
        />
      </template>
    </template>
  </q-breadcrumbs>

  <div class="q-pa-md">
    <q-virtual-scroll
      type="table"
      style="max-height: 70vh"
      :virtual-scroll-item-size="42"
      virtual-scroll-sticky-size-start="32"
      virtual-scroll-sticky-size-end="32"
      :items="files"
      dense
    >
      <template v-slot:before>
        <thead class="thead-sticky text-left">
          <tr>
            <th class="text-center">Mark</th>
            <th class="text-center">Type</th>
            <th class="text-left stretch">Name</th>
          </tr>
        </thead>
      </template>
      <template v-slot="{ item: file, index }">
        <tr :key="index">
          <td>
            <q-checkbox
              v-model="file.marked"
              @update:modelValue="
                (value, event) => updateMarkedStatus(value, file)
              "
            />
          </td>
          <td>
            <q-btn
              v-if="isDirectory(file)"
              @click="(event) => changeDirectory(file, event)"
              :icon="getIcon(file.type)"
              flat
            />
            <q-btn
              v-else
              @click="(event) => event.preventDefault()"
              :icon="getIcon(file.type)"
              flat
            />
          </td>
          <td class="text-flow">
            {{ file.name }}
          </td>
        </tr>
      </template>
    </q-virtual-scroll>
  </div>
</template>

<style scoped lang="sass">
.thead-sticky tr > *,
.tfoot-sticky tr > *
  position: sticky
  opacity: 1
  z-index: 1
  background: black
  color: white

.thead-sticky tr:last-child > *
  top: 0

.tfoot-sticky tr:first-child > *
  bottom: 0

th.stretch
  width: 100%
  max-width: 100%


td.text-flow
  word-wrap: break-word
  white-space: normal
</style>
