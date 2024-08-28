<script setup lang="ts">
import { useWizardStore } from 'src/stores/wizardStore';
import { computed, ref, watch } from 'vue';
import { FileType, File } from 'src/generated/restore';

const wizard = useWizardStore();
const checkedFiles = ref([]);

const getIcon = (type: FileType) => {
  switch (type) {
    case FileType.DIRECTORY:
      return 'folder';
    case FileType.FILE:
      return 'file';
    case FileType.DIRECTORY_NOT_BACKED_UP:
      return 'folder';
    default:
      return 'square-question';
  }
};

const breadcrumbs = computed(() => {
  console.debug('breadcrumbs', wizard.cwd);
  return wizard.cwd;
});

const changeDirectory = (name: File, event: Event) => {
  event.preventDefault();
  console.debug('changeDirectory:', name);
  wizard.changeDirectory(name);
};

const isRowSelectable = (row: File) => {
  console.debug('isRowSelectable:', row);
  console.debug('isRowSelectable:', row.type !== FileType.FILE);
  return row.type !== FileType.FILE;
};

const selected = ref<File>();

watch(selected, (file: File | undefined) => {
  console.debug('selected:', file);
  if (!file) {
    console.info('file is undefined');
    return;
  }
  wizard.changeDirectory(file);
});

const updateMarkedStatus = async (value: boolean, file: File) => {
  console.log('updateMarking:', value, file);
  await wizard.updateMarkedStatus(file);
};
</script>

<template>
  <nav class="breadcrumb" aria-label="breadcrumbs">
    <ul>
      <template v-for="(breadcrumb, index) in breadcrumbs" :key="index">
        <li :class="{ 'is-active': index === breadcrumbs!.length - 1 }">
          <a href="#" @click="(event) => changeDirectory(breadcrumb, event)">
            <template v-if="breadcrumb.id?.value != 0n">
              {{ breadcrumb.name }}
            </template>
            <template v-else>
              <o-icon icon="house" />
            </template>
          </a>
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
    pagination-position="top"
  >
    <o-table-column field="marked" label="Marked" width="40" v-slot="props">
      <o-checkbox
        v-model="props.row.marked"
        @update:modelValue="(value: boolean) => updateMarkedStatus(value as boolean, props.row)"
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
