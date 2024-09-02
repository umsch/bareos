// @generated by protobuf-ts 2.9.4 with parameter generate_dependencies
// @generated from protobuf file "restore.proto" (package "bareos.restore", syntax proto3)
// tslint:disable
//
//
// LICENSE HERE
//
import type { RpcTransport } from "@protobuf-ts/runtime-rpc";
import type { ServiceInfo } from "@protobuf-ts/runtime-rpc";
import { Restore } from "./restore";
import type { CancelResponse } from "./restore";
import type { CancelRequest } from "./restore";
import type { RunResponse } from "./restore";
import type { RunRequest } from "./restore";
import type { CurrentDirectoryResponse } from "./restore";
import type { CurrentDirectoryRequest } from "./restore";
import type { PathToFileResponse } from "./restore";
import type { PathToFileRequest } from "./restore";
import type { ChangeMarkedResponse } from "./restore";
import type { ChangeMarkedRequest } from "./restore";
import type { ChangeDirectoryResponse } from "./restore";
import type { ChangeDirectoryRequest } from "./restore";
import type { File } from "./restore";
import type { ListFilesRequest } from "./restore";
import type { ServerStreamingCall } from "@protobuf-ts/runtime-rpc";
import type { UpdateStateResponse } from "./restore";
import type { UpdateStateRequest } from "./restore";
import type { GetStateResponse } from "./restore";
import type { GetStateRequest } from "./restore";
import type { BeginResponse } from "./restore";
import type { BeginRequest } from "./restore";
import { stackIntercept } from "@protobuf-ts/runtime-rpc";
import type { ListSessionsResponse } from "./restore";
import type { ListSessionsRequest } from "./restore";
import type { UnaryCall } from "@protobuf-ts/runtime-rpc";
import type { RpcOptions } from "@protobuf-ts/runtime-rpc";
/**
 * @generated from protobuf service bareos.restore.Restore
 */
export interface IRestoreClient {
    /**
     * @generated from protobuf rpc: ListSessions(bareos.restore.ListSessionsRequest) returns (bareos.restore.ListSessionsResponse);
     */
    listSessions(input: ListSessionsRequest, options?: RpcOptions): UnaryCall<ListSessionsRequest, ListSessionsResponse>;
    /**
     * @generated from protobuf rpc: Begin(bareos.restore.BeginRequest) returns (bareos.restore.BeginResponse);
     */
    begin(input: BeginRequest, options?: RpcOptions): UnaryCall<BeginRequest, BeginResponse>;
    /**
     * @generated from protobuf rpc: GetState(bareos.restore.GetStateRequest) returns (bareos.restore.GetStateResponse);
     */
    getState(input: GetStateRequest, options?: RpcOptions): UnaryCall<GetStateRequest, GetStateResponse>;
    /**
     * @generated from protobuf rpc: UpdateState(bareos.restore.UpdateStateRequest) returns (bareos.restore.UpdateStateResponse);
     */
    updateState(input: UpdateStateRequest, options?: RpcOptions): UnaryCall<UpdateStateRequest, UpdateStateResponse>;
    /**
     * these functions will only work once the tree is created
     * todo: how do plugin files work
     *
     * @generated from protobuf rpc: ListFiles(bareos.restore.ListFilesRequest) returns (stream bareos.restore.File);
     */
    listFiles(input: ListFilesRequest, options?: RpcOptions): ServerStreamingCall<ListFilesRequest, File>;
    /**
     * @generated from protobuf rpc: ChangeDirectory(bareos.restore.ChangeDirectoryRequest) returns (bareos.restore.ChangeDirectoryResponse);
     */
    changeDirectory(input: ChangeDirectoryRequest, options?: RpcOptions): UnaryCall<ChangeDirectoryRequest, ChangeDirectoryResponse>;
    /**
     * @generated from protobuf rpc: ChangeMarkedStatus(bareos.restore.ChangeMarkedRequest) returns (bareos.restore.ChangeMarkedResponse);
     */
    changeMarkedStatus(input: ChangeMarkedRequest, options?: RpcOptions): UnaryCall<ChangeMarkedRequest, ChangeMarkedResponse>;
    /**
     * @generated from protobuf rpc: PathToFile(bareos.restore.PathToFileRequest) returns (bareos.restore.PathToFileResponse);
     */
    pathToFile(input: PathToFileRequest, options?: RpcOptions): UnaryCall<PathToFileRequest, PathToFileResponse>;
    /**
     * @generated from protobuf rpc: CurrentDirectory(bareos.restore.CurrentDirectoryRequest) returns (bareos.restore.CurrentDirectoryResponse);
     */
    currentDirectory(input: CurrentDirectoryRequest, options?: RpcOptions): UnaryCall<CurrentDirectoryRequest, CurrentDirectoryResponse>;
    // maybe list marked files as function

    /**
     * @generated from protobuf rpc: Run(bareos.restore.RunRequest) returns (bareos.restore.RunResponse);
     */
    run(input: RunRequest, options?: RpcOptions): UnaryCall<RunRequest, RunResponse>;
    /**
     * @generated from protobuf rpc: Cancel(bareos.restore.CancelRequest) returns (bareos.restore.CancelResponse);
     */
    cancel(input: CancelRequest, options?: RpcOptions): UnaryCall<CancelRequest, CancelResponse>;
}
/**
 * @generated from protobuf service bareos.restore.Restore
 */
export class RestoreClient implements IRestoreClient, ServiceInfo {
    typeName = Restore.typeName;
    methods = Restore.methods;
    options = Restore.options;
    constructor(private readonly _transport: RpcTransport) {
    }
    /**
     * @generated from protobuf rpc: ListSessions(bareos.restore.ListSessionsRequest) returns (bareos.restore.ListSessionsResponse);
     */
    listSessions(input: ListSessionsRequest, options?: RpcOptions): UnaryCall<ListSessionsRequest, ListSessionsResponse> {
        const method = this.methods[0], opt = this._transport.mergeOptions(options);
        return stackIntercept<ListSessionsRequest, ListSessionsResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: Begin(bareos.restore.BeginRequest) returns (bareos.restore.BeginResponse);
     */
    begin(input: BeginRequest, options?: RpcOptions): UnaryCall<BeginRequest, BeginResponse> {
        const method = this.methods[1], opt = this._transport.mergeOptions(options);
        return stackIntercept<BeginRequest, BeginResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: GetState(bareos.restore.GetStateRequest) returns (bareos.restore.GetStateResponse);
     */
    getState(input: GetStateRequest, options?: RpcOptions): UnaryCall<GetStateRequest, GetStateResponse> {
        const method = this.methods[2], opt = this._transport.mergeOptions(options);
        return stackIntercept<GetStateRequest, GetStateResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: UpdateState(bareos.restore.UpdateStateRequest) returns (bareos.restore.UpdateStateResponse);
     */
    updateState(input: UpdateStateRequest, options?: RpcOptions): UnaryCall<UpdateStateRequest, UpdateStateResponse> {
        const method = this.methods[3], opt = this._transport.mergeOptions(options);
        return stackIntercept<UpdateStateRequest, UpdateStateResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * these functions will only work once the tree is created
     * todo: how do plugin files work
     *
     * @generated from protobuf rpc: ListFiles(bareos.restore.ListFilesRequest) returns (stream bareos.restore.File);
     */
    listFiles(input: ListFilesRequest, options?: RpcOptions): ServerStreamingCall<ListFilesRequest, File> {
        const method = this.methods[4], opt = this._transport.mergeOptions(options);
        return stackIntercept<ListFilesRequest, File>("serverStreaming", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: ChangeDirectory(bareos.restore.ChangeDirectoryRequest) returns (bareos.restore.ChangeDirectoryResponse);
     */
    changeDirectory(input: ChangeDirectoryRequest, options?: RpcOptions): UnaryCall<ChangeDirectoryRequest, ChangeDirectoryResponse> {
        const method = this.methods[5], opt = this._transport.mergeOptions(options);
        return stackIntercept<ChangeDirectoryRequest, ChangeDirectoryResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: ChangeMarkedStatus(bareos.restore.ChangeMarkedRequest) returns (bareos.restore.ChangeMarkedResponse);
     */
    changeMarkedStatus(input: ChangeMarkedRequest, options?: RpcOptions): UnaryCall<ChangeMarkedRequest, ChangeMarkedResponse> {
        const method = this.methods[6], opt = this._transport.mergeOptions(options);
        return stackIntercept<ChangeMarkedRequest, ChangeMarkedResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: PathToFile(bareos.restore.PathToFileRequest) returns (bareos.restore.PathToFileResponse);
     */
    pathToFile(input: PathToFileRequest, options?: RpcOptions): UnaryCall<PathToFileRequest, PathToFileResponse> {
        const method = this.methods[7], opt = this._transport.mergeOptions(options);
        return stackIntercept<PathToFileRequest, PathToFileResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: CurrentDirectory(bareos.restore.CurrentDirectoryRequest) returns (bareos.restore.CurrentDirectoryResponse);
     */
    currentDirectory(input: CurrentDirectoryRequest, options?: RpcOptions): UnaryCall<CurrentDirectoryRequest, CurrentDirectoryResponse> {
        const method = this.methods[8], opt = this._transport.mergeOptions(options);
        return stackIntercept<CurrentDirectoryRequest, CurrentDirectoryResponse>("unary", this._transport, method, opt, input);
    }
    // maybe list marked files as function

    /**
     * @generated from protobuf rpc: Run(bareos.restore.RunRequest) returns (bareos.restore.RunResponse);
     */
    run(input: RunRequest, options?: RpcOptions): UnaryCall<RunRequest, RunResponse> {
        const method = this.methods[9], opt = this._transport.mergeOptions(options);
        return stackIntercept<RunRequest, RunResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: Cancel(bareos.restore.CancelRequest) returns (bareos.restore.CancelResponse);
     */
    cancel(input: CancelRequest, options?: RpcOptions): UnaryCall<CancelRequest, CancelResponse> {
        const method = this.methods[10], opt = this._transport.mergeOptions(options);
        return stackIntercept<CancelRequest, CancelResponse>("unary", this._transport, method, opt, input);
    }
}
