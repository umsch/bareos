import { useGrpcStore } from 'stores/grpcStore'

import {
  type File,
  FileType,
  MarkAction,
  RestoreOptions,
  type RestoreSession,
  StartSelection,
} from 'src/generated/restore'
import { IRestoreClient } from 'src/generated/restore.client'

export class Restore {
  private readonly restoreClient: IRestoreClient

  public constructor() {
    const grpcStore = useGrpcStore()
    this.restoreClient = grpcStore.restoreClient!
  }

  async fetchSessions() {
    const response = await this.restoreClient.listSessions({})
    return response!.response.sessions!
  }

  async createSession(startSelection: StartSelection) {
    const response = await this.restoreClient.begin({
      start: startSelection,
    })
    return response?.response.session
  }

  async deleteSession(session: RestoreSession) {
    await this.restoreClient.cancel({ session })
  }

  async runSession(session: RestoreSession) {
    try {
      await this.restoreClient.run({
        session,
      })
    } catch (e) {
      console.error(e)
    }

    console.debug('session end')
  }

  async fetchFiles(session: RestoreSession) {
    const files: File[] = []
    try {
      const call = this.restoreClient.listFiles({ session: session })
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

  async currentDirectory(session: RestoreSession) {
    const response = await this.restoreClient.currentDirectory({ session })
    return response?.response.currentDir?.segments
  }

  async changeDirectory(session: RestoreSession, path: File) {
    if (
      path.type !== FileType.DIRECTORY &&
      path.type !== FileType.DIRECTORY_NOT_BACKED_UP
    ) {
      throw new Error('Invalid file type. File type must be a directory')
    }

    const response = await this.restoreClient.changeDirectory({
      session: session,
      directory: path.id,
    })
    return response?.response.currentDir?.segments
  }

  async changeMarkedStatus(session: RestoreSession, file: File, mark: boolean) {
    await this.restoreClient.changeMarkedStatus({
      session: session,
      action: mark ? MarkAction.MARK : MarkAction.UNMARK,
      affectedId: file.id!,
      recursive:
        file.type === FileType.DIRECTORY ||
        file.type === FileType.DIRECTORY_NOT_BACKED_UP,
    })
  }

  async fetchState(session: RestoreSession) {
    const response = await this.restoreClient.getState({ session })

    console.log(response?.response.state ?? null)
    return response?.response.state ?? null
  }

  async pushRestoreOptions(
    session: RestoreSession,
    newOptions: RestoreOptions,
  ) {
    return this.restoreClient.updateState({ session, newOptions })
  }
}
