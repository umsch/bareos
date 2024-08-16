<script setup lang="ts">
import { useWizardStore } from '@/stores/wizardStore'
import { computed, ref, watch } from 'vue'
import { OCheckbox, OTableColumn } from '@oruga-ui/oruga-next'
import { FileType, File } from '@/generated/restore'

const wizard = useWizardStore()
const columns = ref([
  {
    field: 'marked',
    label: 'Marked',
    width: '1%',
    type: 'checkbox'
  },
  {
    field: 'type',
    label: 'Type',
    width: '1%'
  },
  {
    field: 'name',
    label: 'Name',
    searchable: true
  }
])

const checkedFiles = ref([])

const getIcon = (type: FileType) => {
  switch (type) {
    case FileType.DIRECTORY:
      return 'folder'
    case FileType.FILE:
      return 'file'
    case FileType.DIRECTORY_NOT_BACKED_UP:
      return 'folder'
    default:
      return 'square-question'
  }
}

function splitPath(dirPath: string): Array<{ name: string; path: string }> {
  if (!dirPath || dirPath.length === 0) {
    return []
  }

  const parts = dirPath.split('/').filter(Boolean)
  let currentPath = ''

  const result = parts.map((part, index) => {
    currentPath += '/' + part
    return {
      name: index === 0 ? 'root' : part,
      path: index === 0 ? '/' : currentPath
    }
  })

  return [{ name: 'root', path: '/' }, ...result.slice(1)]
}

const breadcrumbs = computed(() => {
  console.debug('wizard.cwd:', wizard.cwd)
  return splitPath(wizard.cwd || '')
})

const changeDirectory = (name: string) => {
  console.debug('changeDirectory:', name)
  wizard.changeDirectory(name)
}

const isRowSelectable = (row: any) => {
  console.debug('isRowSelectable:', row)
  console.debug('isRowSelectable:', row.type !== FileType.FILE)
  return row.type !== FileType.FILE
}

const selected = ref<File>()

watch(selected, (value) => {
  console.debug('selected:', value)
  wizard.changeDirectory(value?.name ?? '')
})

const updateMarkedStatus = async (value: boolean, file: File) => {
  console.log('updateMarking:', value, file)
  await wizard.updateMarkedStatus(file)
}
</script>

<template>
  <nav class="breadcrumb" aria-label="breadcrumbs">
    <ul>
      <template v-for="(breadcrumb, index) in breadcrumbs" :key="index">
        <li :class="{ 'is-active': index === breadcrumbs!.length - 1 }">
          <a href="#" @click="changeDirectory(breadcrumb.path)">{{ breadcrumb.name }}</a>
        </li>
      </template>
    </ul>
  </nav>

  <o-table
    :data="wizard.files"
    v-model:checked-rows="checkedFiles"
    paginated
    per-page="20"
    :isRowSelectable="isRowSelectable"
    v-model:selected="selected"
    narrowed
  >
    <o-table-column field="marked" label="Marked" width="40" v-slot="props">
      <o-checkbox
        v-model="props.row.marked"
        @update:modelValue="(value) => updateMarkedStatus(value as boolean, props.row)"
        size="normal"
      />
    </o-table-column>
    <o-table-column field="type" label="Type" width="40" v-slot="props">
      <o-icon :icon="getIcon(props.row.type)"></o-icon>
    </o-table-column>
    <o-table-column field="name" label="Name" v-slot="props" searchable>
      {{ props.row.name }}
    </o-table-column>
  </o-table>
</template>

<style scoped></style>
