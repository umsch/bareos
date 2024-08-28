import { defineStore } from 'pinia';
import type { Catalog, CatalogId } from 'src/generated/config';
import { onMounted, ref, watch } from 'vue';
import type { Client, Job } from 'src/generated/common';
import type { RestoreSession, File } from 'src/generated/restore';
import { useRestoreClientStore } from 'src/stores/restoreClientStore';
import { first, isEmpty, reverse } from 'lodash';

export const useWizardStore = defineStore('wizard', () => {
  const restoreClient = useRestoreClientStore();

  onMounted(async () => {
    await updateCatalogs();
    await updateSessions();

    if (!selectedCatalog.value && !isEmpty(catalogs.value)) {
      selectedCatalog.value = first(catalogs.value)!;
    }

    if (!selectedSession.value && !isEmpty(sessions.value)) {
      selectedSession.value = first(sessions.value)!;
    }
  });

  // clients
  const catalogs = ref<Catalog[]>([]);
  const isCatalogsLoading = ref(false);
  const selectedCatalog = ref<Catalog | null>(null);
  const updateCatalogs = async () => {
    isCatalogsLoading.value = true;
    try {
      console.debug('fetching catalogs');
      catalogs.value = await restoreClient.fetchCatalogs();
    } finally {
      isCatalogsLoading.value = false;
    }
  };

  // jobs
  const selectedJob = ref<Job | null>(null);
  const jobs = ref<Job[]>([]);
  const isJobsLoading = ref(false);
  const updateJobs = async (catalog_id: CatalogId) => {
    isJobsLoading.value = true;
    try {
      console.debug('fetching jobs from catalog', catalog_id);
      jobs.value = await restoreClient.fetchJobs(catalog_id);
    } finally {
      isJobsLoading.value = false;
    }
  };

  // sessions
  const selectedSession = ref<RestoreSession | null>(null);
  const sessions = ref<RestoreSession[]>([]);
  const isSessionsLoading = ref(false);

  const updateSessions = async () => {
    try {
      console.debug('fetching sessions');
      isSessionsLoading.value = true;
      sessions.value = await restoreClient.fetchSessions();
    } finally {
      isSessionsLoading.value = false;
    }
  };

  // client
  const clients = ref<Client[]>([]);
  const selectedClient = ref<Client | null>(null);
  const updateClients = async (catalog: Catalog) => {
    if (!selectedCatalog.value) {
      throw new Error('No catalog selected');
    }

    clients.value = await restoreClient.fetchClients(catalog);
  };

  // logics

  watch(selectedCatalog, async (catalog) => {
    console.debug('selected catalog changed', catalog);
    if (catalog?.id) {
      await updateJobs(catalog.id);
      await updateClients(catalog);
    }
  });

  watch(selectedJob, async () => {
    await cleanUpSessions();
    await createRestoreSession();
    await updateSessions();
  });

  const cleanUpSessions = async () => {
    selectedSession.value = null;

    if (!isEmpty(sessions.value)) {
      for (const session of sessions.value) {
        await restoreClient.deleteSession(session);
      }
    }

    await updateSessions();
  };

  const createRestoreSession = async () => {
    console.debug('starting restore session', selectedJob.value?.jobid);

    if (!selectedJob.value) {
      throw new Error('No job selected');
    }

    const session = await restoreClient.createSession(selectedJob.value);
    await updateSessions();
    selectedSession.value = session!;
  };

  const runRestoreSession = async () => {
    if (!selectedClient.value) {
      throw new Error('No client selected');
    }

    if (!selectedSession.value) {
      throw new Error('No session selected');
    }

    await restoreClient.runSession(
      selectedSession.value!,
      selectedClient.value
    );
  };

  const deleteRestoreSession = async () => {
    if (!selectedSession.value) {
      return;
    }

    await restoreClient.deleteSession(selectedSession.value);
    selectedSession.value = null;
    await updateSessions();
  };

  const files = ref<File[]>([]);
  const cwd = ref<File[] | undefined>([]);

  watch(selectedSession, async (session) => {
    console.debug('selected session changed', session);
    console.debug('updating path and files');

    if (session) {
      const currentPath = await restoreClient.currentDirectory(session);
      cwd.value = reverse(currentPath!);
      files.value = await restoreClient.fetchFiles(session);
    } else {
      cwd.value = undefined;
      files.value = [];
    }
  });

  const changeDirectory = async (path: File) => {
    if (!selectedSession.value) {
      throw new Error('No session selected');
    }

    const currentPath = await restoreClient.changeDirectory(
      selectedSession.value,
      path
    );
    cwd.value = reverse(currentPath!);
    files.value = await restoreClient.fetchFiles(selectedSession.value);
  };

  const updateMarkedStatus = async (file: File) => {
    if (!selectedSession.value) {
      throw new Error('No session selected');
    }

    await restoreClient.changeMarkedStatus(
      selectedSession.value,
      file,
      file.marked
    );
  };

  return {
    updateCatalogs,
    catalogs,
    selectedCatalog,
    clients,
    selectedClient,
    updateJobs,
    isJobsLoading,
    jobs,
    selectedJob,
    updateSessions,
    isSessionsLoading,
    sessions,
    selectedSession,
    createRestoreSession,
    runRestoreSession,
    deleteRestoreSession,
    files,
    cwd,
    changeDirectory,
    updateMarkedStatus,
  };
});
