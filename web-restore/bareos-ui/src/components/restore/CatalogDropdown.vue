<script setup lang="ts">
import { defineProps, defineEmits, ref, watch } from 'vue';
import type { Catalog } from '@/generated/config';

const props = defineProps<{
  catalogs: Catalog[];
  initialSelection: Catalog | null;
}>();

const emit = defineEmits<{
  (e: 'update:selectedCatalog', catalog: Catalog): void;
}>();

const selected = ref<Catalog | null>(props.initialSelection);

watch(selected, (newValue) => {
  if (newValue) {
    emit('update:selectedCatalog', newValue);
  }
});

</script>
<template>
  <o-field label="Catalog">
    <o-dropdown v-model="selected" expanded>
      <template #trigger="{ active }">
        <o-button
          variant="primary"
          :label="selected ? selected.name : 'Select Catalog'"
          :icon-right="active ? 'caret-up' : 'caret-down'" />
      </template>

      <o-dropdown-item
        v-for="(catalog, index) in catalogs"
        :key="index"
        :value="catalog">
        <div>
          <div>{{ catalog.name }}</div>
        </div>
      </o-dropdown-item>

    </o-dropdown>
  </o-field>
</template>


