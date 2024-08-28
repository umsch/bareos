<script setup lang="ts">
import { onBeforeMount, ref, watch } from 'vue';

import { useWizardStore } from 'src/stores/wizardStore';
import { QSelect } from 'quasar';
import { Job } from 'src/generated/common';

const wizard = useWizardStore();
const selection = ref<Job | null>();

onBeforeMount(() => {
  selection.value = wizard.selectedJob;
});

watch(selection, (s) => (wizard.selectedJob = s ?? null));

const options = ref(wizard.jobs);

const filter = (
  val: string,
  update: (fn: () => void, ref?: (ref: QSelect) => void) => void
) => {
  if (val === '') {
    update(() => {
      options.value = wizard.jobs;
    });
  } else {
    setTimeout(() => {
      update(
        () => {
          console.debug('value', val);
          const needle = val.toLowerCase();
          options.value = wizard.jobs.filter(
            (v) => v.jobid.toString().toLowerCase().indexOf(needle) > -1
          );
        },
        (ref) => {
          if (ref.options || (val !== '' && ref.options!.length > 0)) {
            ref.setOptionIndex(-1); // reset optionIndex in case there is something selected
            ref.moveOptionSelection(1, true); // focus the first selectable option and do not update the input-value
          }
        }
      );
    }, 300);
  }
};
</script>

<template>
  <q-select
    filled
    v-model="selection"
    clearable
    use-input
    hide-selected
    fill-input
    input-debounce="0"
    label="Backup Job"
    :options="wizard.jobs"
    option-label="jobid"
    @filter="filter"
  >
    <template v-slot:no-option>
      <q-item>
        <q-item-section class="text-grey"> No results </q-item-section>
      </q-item>
    </template>
  </q-select>
</template>

<style scoped></style>
