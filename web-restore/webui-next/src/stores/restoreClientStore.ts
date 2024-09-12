import { ref } from 'vue'

import { defineStore, storeToRefs } from 'pinia'

import { useGrpcStore } from 'stores/grpcStore'

import type { Catalog } from 'src/generated/config'
import {
  type File,
  FileType,
  MarkAction,
  RestoreOptions,
  type RestoreSession,
  StartSelection,
} from 'src/generated/restore'

export const useRestoreClientStore = defineStore('restore-client', () => {
  const { restoreClient } = storeToRefs(useGrpcStore())
  const catalogs = ref<Catalog[]>([])

  const fetchSessions = async () => {
    const response = await restoreClient.value?.listSessions({})
    return response!.response.sessions!
  }

  const createSession = async (startSelection: StartSelection) => {
    const response = await restoreClient.value?.begin({
      start: startSelection,
    })
    return response?.response.session
  }

  const deleteSession = async (session: RestoreSession) => {
    await restoreClient.value?.cancel({ session })
  }

  const runSession = async (session: RestoreSession) => {
    try {
      await restoreClient.value?.run({
        session,
      })
    } catch (e) {
      console.error(e)
    }

    console.debug('session end')
  }

  const fetchFiles = async (session: RestoreSession) => {
    const files: File[] = []
    try {
      const call = restoreClient.value?.listFiles({ session: session })
      if (!call?.responses) {
        return files
      }
      for await (const file of call.responses) {
        files.push(file)
      }
    } catch (e) {
      console.error(e)
    }
    return files
  }

  const currentDirectory = async (session: RestoreSession) => {
    const response = await restoreClient.value?.currentDirectory({ session })
    return response?.response.currentDir?.segments
  }

  const changeDirectory = async (session: RestoreSession, path: File) => {
    if (
      path.type !== FileType.DIRECTORY &&
      path.type !== FileType.DIRECTORY_NOT_BACKED_UP
    ) {
      throw new Error('Invalid file type. File type must be a directory')
    }

    const response = await restoreClient.value?.changeDirectory({
      session: session,
      directory: path.id,
    })
    return response?.response.currentDir?.segments
  }

  const changeMarkedStatus = async (
    session: RestoreSession,
    file: File,
    mark: boolean,
  ) => {
    await restoreClient.value?.changeMarkedStatus({
      session: session,
      action: mark ? MarkAction.MARK : MarkAction.UNMARK,
      affectedId: file.id!,
      recursive:
        file.type === FileType.DIRECTORY ||
        file.type === FileType.DIRECTORY_NOT_BACKED_UP,
    })
  }

  const fetchState = async (session: RestoreSession) => {
    const response = await restoreClient.value?.getState({ session })

    console.log(response?.response.state ?? null)
    return response?.response.state ?? null
  }

  const pushRestoreOptions = async (
    session: RestoreSession,
    newOptions: RestoreOptions,
  ) => {
    return restoreClient.value?.updateState({ session, newOptions })
  }

  return {
    catalogs,
    fetchSessions,
    createSession,
    deleteSession,
    runSession,
    fetchFiles,
    currentDirectory,
    changeDirectory,
    changeMarkedStatus,
    fetchState,
    pushRestoreOptions,
  }
})
