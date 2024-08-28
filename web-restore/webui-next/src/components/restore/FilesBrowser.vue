<script setup lang="ts">
import { useWizardStore } from 'src/stores/wizardStore'
import { computed, ref, watch } from 'vue'
import { File, FileType } from 'src/generated/restore'
import {
  matQuestionMark,
  matInsertDriveFile,
  matFolder,
} from '@quasar/extras/material-icons'

const wizard = useWizardStore()
// const checkedFiles = ref([]);

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

const breadcrumbs = computed(() => {
  console.debug('breadcrumbs', wizard.cwd)
  return wizard.cwd
})

const changeDirectory = (name: File, event: Event) => {
  event.preventDefault()
  console.debug('changeDirectory:', name)
  wizard.changeDirectory(name)
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
  wizard.changeDirectory(file)
})

// const updateMarkedStatus = async (value: boolean, file: File) => {
//   console.log('updateMarking:', value, file);
//   await wizard.updateMarkedStatus(file);
// };
</script>

<template>
  <q-breadcrumbs gutter="sm">
    <template v-for="(breadcrumb, index) in breadcrumbs" :key="index">
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
      :items="wizard.files"
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
      <template v-slot="{ item: row, index }">
        <tr :key="index">
          <td>
            <q-checkbox v-model="row.marked" />
          </td>
          <td>
            <q-btn
              v-if="isDirectory(row)"
              @click="(event) => changeDirectory(row, event)"
              :icon="getIcon(row.type)"
              flat
            />
            <q-btn
              v-else
              @click="(event) => event.preventDefault()"
              :icon="getIcon(row.type)"
              flat
            />
          </td>
          <td class="text-flow">
            {{ row.name }}
          </td>
        </tr>
      </template>
    </q-virtual-scroll>
  </div>
</template>

<style scoped lang="scss">
table {
  width: 100%;
}

th.stretch {
  width: 100%;
  max-width: 100%;
}

.overflow-hidden {
  overflow-x: hidden;
}

.text-flow {
  white-space: nowrap;
  text-overflow: ellipsis;
}
</style>
