<script setup lang="ts">
import { onBeforeMount, ref } from 'vue'
import { useWizardStore } from 'src/stores/wizardStore'

import { QSelect } from 'quasar'

const wizard = useWizardStore()

const searchString = ref<string>()
const options = ref(wizard.clients)

const filter = (
  val: string,
  update: (fn: () => void, ref?: (ref: QSelect) => void) => void
) => {
  if (val === '') {
    update(() => {
      options.value = wizard.clients
    })
  } else {
    setTimeout(() => {
      update(
        () => {
          console.debug('value', val)
          const needle = val.toLowerCase()
          options.value = wizard.clients.filter(
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
  searchString.value = wizard.selectedClient?.name
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
    label="Client"
    :options="wizard.clients"
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
