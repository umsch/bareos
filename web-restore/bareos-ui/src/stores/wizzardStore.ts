import { defineStore } from 'pinia'
import type { Catalog } from '@/generated/config'
import { ref } from 'vue'
import type { Client, Job } from '@/generated/common'

export const useWizzardStore = defineStore('wizzard', () => {
  const selectedCatalog = ref<Catalog | null>(null)
  const selectedClient = ref<Client | null>(null)
  const selectedJob = ref<Job | null>(null)

  return { selectedCatalog, selectedClient, selectedJob }
})
