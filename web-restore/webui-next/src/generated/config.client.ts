// @generated by protobuf-ts 2.9.4 with parameter generate_dependencies
// @generated from protobuf file "config.proto" (package "bareos.config", syntax proto3)
// tslint:disable
//
//
// LICENSE HERE
//
import type { RpcTransport } from "@protobuf-ts/runtime-rpc";
import type { ServiceInfo } from "@protobuf-ts/runtime-rpc";
import { Config } from "./config";
import type { GetDefinitionResponse } from "./config";
import type { GetDefinitionRequest } from "./config";
import type { ListCatalogsResponse } from "./config";
import type { ListCatalogsRequest } from "./config";
import type { ListJobsResponse } from "./config";
import type { ListJobsRequest } from "./config";
import { stackIntercept } from "@protobuf-ts/runtime-rpc";
import type { ListClientsResponse } from "./config";
import type { ListClientsRequest } from "./config";
import type { UnaryCall } from "@protobuf-ts/runtime-rpc";
import type { RpcOptions } from "@protobuf-ts/runtime-rpc";
/**
 * @generated from protobuf service bareos.config.Config
 */
export interface IConfigClient {
    /**
     * @generated from protobuf rpc: ListClients(bareos.config.ListClientsRequest) returns (bareos.config.ListClientsResponse);
     */
    listClients(input: ListClientsRequest, options?: RpcOptions): UnaryCall<ListClientsRequest, ListClientsResponse>;
    /**
     * @generated from protobuf rpc: ListJobs(bareos.config.ListJobsRequest) returns (bareos.config.ListJobsResponse);
     */
    listJobs(input: ListJobsRequest, options?: RpcOptions): UnaryCall<ListJobsRequest, ListJobsResponse>;
    /**
     * @generated from protobuf rpc: ListCatalogs(bareos.config.ListCatalogsRequest) returns (bareos.config.ListCatalogsResponse);
     */
    listCatalogs(input: ListCatalogsRequest, options?: RpcOptions): UnaryCall<ListCatalogsRequest, ListCatalogsResponse>;
    /**
     * @generated from protobuf rpc: GetDefinition(bareos.config.GetDefinitionRequest) returns (bareos.config.GetDefinitionResponse);
     */
    getDefinition(input: GetDefinitionRequest, options?: RpcOptions): UnaryCall<GetDefinitionRequest, GetDefinitionResponse>;
}
/**
 * @generated from protobuf service bareos.config.Config
 */
export class ConfigClient implements IConfigClient, ServiceInfo {
    typeName = Config.typeName;
    methods = Config.methods;
    options = Config.options;
    constructor(private readonly _transport: RpcTransport) {
    }
    /**
     * @generated from protobuf rpc: ListClients(bareos.config.ListClientsRequest) returns (bareos.config.ListClientsResponse);
     */
    listClients(input: ListClientsRequest, options?: RpcOptions): UnaryCall<ListClientsRequest, ListClientsResponse> {
        const method = this.methods[0], opt = this._transport.mergeOptions(options);
        return stackIntercept<ListClientsRequest, ListClientsResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: ListJobs(bareos.config.ListJobsRequest) returns (bareos.config.ListJobsResponse);
     */
    listJobs(input: ListJobsRequest, options?: RpcOptions): UnaryCall<ListJobsRequest, ListJobsResponse> {
        const method = this.methods[1], opt = this._transport.mergeOptions(options);
        return stackIntercept<ListJobsRequest, ListJobsResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: ListCatalogs(bareos.config.ListCatalogsRequest) returns (bareos.config.ListCatalogsResponse);
     */
    listCatalogs(input: ListCatalogsRequest, options?: RpcOptions): UnaryCall<ListCatalogsRequest, ListCatalogsResponse> {
        const method = this.methods[2], opt = this._transport.mergeOptions(options);
        return stackIntercept<ListCatalogsRequest, ListCatalogsResponse>("unary", this._transport, method, opt, input);
    }
    /**
     * @generated from protobuf rpc: GetDefinition(bareos.config.GetDefinitionRequest) returns (bareos.config.GetDefinitionResponse);
     */
    getDefinition(input: GetDefinitionRequest, options?: RpcOptions): UnaryCall<GetDefinitionRequest, GetDefinitionResponse> {
        const method = this.methods[3], opt = this._transport.mergeOptions(options);
        return stackIntercept<GetDefinitionRequest, GetDefinitionResponse>("unary", this._transport, method, opt, input);
    }
}
