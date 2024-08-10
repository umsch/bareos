// @generated by protobuf-ts 2.9.4 with parameter generate_dependencies
// @generated from protobuf file "database.proto" (package "bareos.database", syntax proto3)
// tslint:disable
//
//
// LICENSE HERE
//
import type { RpcTransport } from "@protobuf-ts/runtime-rpc";
import type { ServiceInfo } from "@protobuf-ts/runtime-rpc";
import { Database } from "./database";
import type { ListJobsResponse } from "./database";
import type { ListJobsRequest } from "./database";
import { stackIntercept } from "@protobuf-ts/runtime-rpc";
import type { ListClientsResponse } from "./database";
import type { ListClientsRequest } from "./database";
import type { ServerStreamingCall } from "@protobuf-ts/runtime-rpc";
import type { RpcOptions } from "@protobuf-ts/runtime-rpc";
/**
 * Should these functions return a typed response (like now) or rather a generic
 * message SqlResponse { map<string, string>; } ?
 *
 * @generated from protobuf service bareos.database.Database
 */
export interface IDatabaseClient {
    /**
     * @generated from protobuf rpc: ListClients(bareos.database.ListClientsRequest) returns (stream bareos.database.ListClientsResponse);
     */
    listClients(input: ListClientsRequest, options?: RpcOptions): ServerStreamingCall<ListClientsRequest, ListClientsResponse>;
    /**
     * @generated from protobuf rpc: ListJobs(bareos.database.ListJobsRequest) returns (stream bareos.database.ListJobsResponse);
     */
    listJobs(input: ListJobsRequest, options?: RpcOptions): ServerStreamingCall<ListJobsRequest, ListJobsResponse>;
}
/**
 * Should these functions return a typed response (like now) or rather a generic
 * message SqlResponse { map<string, string>; } ?
 *
 * @generated from protobuf service bareos.database.Database
 */
export class DatabaseClient implements IDatabaseClient, ServiceInfo {
    typeName = Database.typeName;
    methods = Database.methods;
    options = Database.options;
    constructor(private readonly _transport: RpcTransport) {
    }
    /**
     * @generated from protobuf rpc: ListClients(bareos.database.ListClientsRequest) returns (stream bareos.database.ListClientsResponse);
     */
    listClients(input: ListClientsRequest, options?: RpcOptions): ServerStreamingCall<ListClientsRequest, ListClientsResponse> {
        const method = this.methods[0], opt = this._transport.mergeOptions(options);
        return stackIntercept<ListClientsRequest, ListClientsResponse>("serverStreaming", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: ListJobs(bareos.database.ListJobsRequest) returns (stream bareos.database.ListJobsResponse);
     */
    listJobs(input: ListJobsRequest, options?: RpcOptions): ServerStreamingCall<ListJobsRequest, ListJobsResponse> {
        const method = this.methods[1], opt = this._transport.mergeOptions(options);
        return stackIntercept<ListJobsRequest, ListJobsResponse>("serverStreaming", this._transport, method, opt, input);
    }
}