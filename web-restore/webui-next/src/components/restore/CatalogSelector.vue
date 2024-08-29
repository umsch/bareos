<script setup lang="ts">
import { onBeforeMount, ref } from 'vue'
import { useWizardStore } from 'src/stores/wizardStore'
import { QSelect } from 'quasar'
import { Catalog } from 'src/generated/config'

const wizard = useWizardStore()

const searchString = ref<Catalog | null>()
const options = ref<Catalog[]>(wizard.catalogs)

const filter = (
  val: string,
  update: (fn: () => void, ref?: (ref: QSelect) => void) => void
) => {
  if (val === '') {
    update(() => {
      options.value = wizard.catalogs
    })
  } else {
    setTimeout(() => {
      update(
        () => {
          console.debug('value', val)
          const needle = val.toLowerCase()
          options.value = wizard.catalogs.filter(
            (v) => v.name.toLowerCase().indexOf(needle) > -1
          )
        },
        (ref) => {
          if (ref.options || (val !== '' && ref.options!.length > 0)) {
            ref.setOptionIndex(-1) // reset optionIndex in case there is something selected
            ref.moveOptionSelection(1, true) // focus the first selectable option and do not update the input-value
          }
        }
      )
    }, 300)
  }
}

onBeforeMount(() => {
  searchString.value = wizard.selectedCatalog
})
</script>
<template>
  <q-select
    filled
    v-model="searchString"
    clearable
    use-input
    hide-selected
    fill-input
    input-debounce="0"
    label="Catalog"
    :options="wizard.catalogs"
    option-label="name"
    @filter="filter"
  >
    <template v-slot:no-option>
      <q-item>
        <q-item-section class="text-grey"> No results </q-item-section>
      </q-item>
    </template>
  </q-select>
</template>
