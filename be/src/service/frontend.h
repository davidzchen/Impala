// Copyright 2012 Cloudera Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef IMPALA_SERVICE_FRONTEND_H
#define IMPALA_SERVICE_FRONTEND_H

#include <jni.h>

#include "gen-cpp/ImpalaService.h"
#include "gen-cpp/ImpalaHiveServer2Service.h"
#include "gen-cpp/ImpalaInternalService.h"
#include "gen-cpp/Frontend_types.h"
#include "common/status.h"

namespace impala {

// The Frontend is a proxy for the Java-side JniFrontend class. The interface is a set of
// wrapper methods for methods called over JNI.
// TODO: Consider changing all methods to accept and return only Thrift structures so that
// all go through exactly the same calling code.
class Frontend {
 public:
  // Does all the work of initialising the JNI method stubs. If any method can't be found,
  // or if there is any further exception, the constructor will terminate the process.
  Frontend();

  // Request to update the Impalad catalog cache. The TUpdateCatalogCacheRequest contains
  // a list of objects that should be added/removed from the Catalog. Returns a response
  // that contains details such as the new max catalog version.
  Status UpdateCatalogCache(const TUpdateCatalogCacheRequest& req,
      TUpdateCatalogCacheResponse *resp);

  // Call FE to get explain plan
  Status GetExplainPlan(const TQueryCtx& query_ctx, std::string* explain_string);

  // Call FE to get TExecRequest.
  Status GetExecRequest(const TQueryCtx& query_ctx, TExecRequest* result);

  // Returns all matching table names, per Hive's "SHOW TABLES <pattern>". Each
  // table name returned is unqualified.
  // If pattern is NULL, match all tables otherwise match only those tables that
  // match the pattern string. Patterns are "p1|p2|p3" where | denotes choice,
  // and each pN may contain wildcards denoted by '*' which match all strings.
  // The TSessionState parameter is used to filter results of metadata operations when
  // authorization is enabled. If this is a user initiated request, it should
  // be set to the user's current session. If this is an Impala internal request,
  // the session should be set to NULL which will skip privilege checks returning all
  // results.
  Status GetTableNames(const std::string& db, const std::string* pattern,
      const TSessionState* session, TGetTablesResult* table_names);

  // Return all databases matching the optional argument 'pattern'.
  // If pattern is NULL, match all databases otherwise match only those databases that
  // match the pattern string. Patterns are "p1|p2|p3" where | denotes choice,
  // and each pN may contain wildcards denoted by '*' which match all strings.
  // The TSessionState parameter is used to filter results of metadata operations when
  // authorization is enabled. If this is a user initiated request, it should
  // be set to the user's current session. If this is an Impala internal request,
  // the session should be set to NULL which will skip privilege checks returning all
  // results.
  Status GetDbNames(const std::string* pattern, const TSessionState* session,
      TGetDbsResult* table_names);

  // Return all data sources matching the optional argument 'pattern'.
  // If pattern is NULL, match all data source names otherwise match only those that
  // match the pattern string. Patterns are "p1|p2|p3" where | denotes choice,
  // and each pN may contain wildcards denoted by '*' which match all strings.
  Status GetDataSrcMetadata(const std::string* pattern, TGetDataSrcsResult* result);

  // Call FE to get the table/column stats.
  Status GetStats(const TShowStatsParams& params, TResultSet* result);

  // Call FE to get the privileges granted to a role.
  Status GetRolePrivileges(const TShowGrantRoleParams& params, TResultSet* result);

  // Return all functions of 'category' that match the optional argument 'pattern'.
  // If pattern is NULL match all functions, otherwise match only those functions that
  // match the pattern string.
  // The TSessionState parameter is used to filter results of metadata operations when
  // authorization is enabled. If this is a user initiated request, it should
  // be set to the user's current session. If this is an Impala internal request,
  // the session should be set to NULL which will skip privilege checks returning all
  // results.
  Status GetFunctions(TFunctionCategory::type fn_category, const std::string& db,
      const std::string* pattern, const TSessionState* session,
      TGetFunctionsResult* functions);

  // Gets the Thrift representation of a Catalog object. The request is a TCatalogObject
  // which has the desired TCatalogObjectType and name properly set.
  // Returns OK if the operation was successful, otherwise a Status object with
  // information on the error will be returned.
  Status GetCatalogObject(const TCatalogObject& request, TCatalogObject* response);

  // Call FE to get the roles.
  Status ShowRoles(const TShowRolesParams& params, TShowRolesResult* result);

  // Returns (in the output parameter) the result of a DESCRIBE table command. This
  // command retrieves table metadata, such as the column definitions. The metadata
  // that is returned is controlled by setting the 'output_style' field. If this
  // field is set to MINIMAL, only the column definitions are returned. If set to
  // FORMATTED, extended metadata is returned (in addition to the column defs).
  // This includes info about the table properties, SerDe properties, StorageDescriptor
  // properties, and more.
  Status DescribeTable(const TDescribeTableParams& params,
      TDescribeTableResult* response);

  // Returns (in the output parameter) a string containing the CREATE TABLE command that
  // creates the table specified in the params.
  Status ShowCreateTable(const TTableName& table_name, std::string* response);

  // Validate Hadoop config; requires FE
  Status ValidateSettings();

  // Calls FE to execute HiveServer2 metadata operation.
  Status ExecHiveServer2MetadataOp(const TMetadataOpRequest& request,
      TResultSet* result);

  // Returns all Hadoop configurations in key, value form in result.
  Status GetAllHadoopConfigs(TGetAllHadoopConfigsResponse* result);

  // Returns (in the output parameter) the value for the given config. The returned Thrift
  // struct will indicate if the value was null or not found by not setting its 'value'
  // field.
  Status GetHadoopConfig(const TGetHadoopConfigRequest& request,
      TGetHadoopConfigResponse* response);

  // Loads a single file or set of files into a table or partition. Saves the RPC
  // response in the TLoadDataResp output parameter. Returns OK if the operation
  // completed successfully.
  Status LoadData(const TLoadDataReq& load_data_request, TLoadDataResp* response);

  // Returns true if the error returned by the FE was due to an AuthorizationException.
  static bool IsAuthorizationError(const Status& status);

  // Sets the FE catalog to be initialized. This is only used for testing in
  // conjunction with InProcessImpalaServer. This sets the FE catalog to
  // be initialized, ready to receive queries without needing a catalog
  // server.
  Status SetCatalogInitialized();

 private:
  // Descriptor of Java Frontend class itself, used to create a new instance.
  jclass fe_class_;

  jobject fe_;  // instance of com.cloudera.impala.service.JniFrontend
  jmethodID create_exec_request_id_;  // JniFrontend.createExecRequest()
  jmethodID get_explain_plan_id_;  // JniFrontend.getExplainPlan()
  jmethodID get_hadoop_config_id_;  // JniFrontend.getHadoopConfig(byte[])
  jmethodID get_hadoop_configs_id_;  // JniFrontend.getAllHadoopConfigs()
  jmethodID check_config_id_; // JniFrontend.checkConfiguration()
  jmethodID update_catalog_cache_id_; // JniFrontend.updateCatalogCache()
  jmethodID get_table_names_id_; // JniFrontend.getTableNames
  jmethodID describe_table_id_; // JniFrontend.describeTable
  jmethodID show_create_table_id_; // JniFrontend.showCreateTable
  jmethodID get_db_names_id_; // JniFrontend.getDbNames
  jmethodID get_data_src_metadata_id_; // JniFrontend.getDataSrcMetadata
  jmethodID get_stats_id_; // JniFrontend.getTableStats
  jmethodID get_functions_id_; // JniFrontend.getFunctions
  jmethodID get_catalog_object_id_; // JniFrontend.getCatalogObject
  jmethodID show_roles_id_; // JniFrontend.getRoles
  jmethodID get_role_privileges_id_; // JniFrontend.getRolePrivileges
  jmethodID exec_hs2_metadata_op_id_; // JniFrontend.execHiveServer2MetadataOp
  jmethodID load_table_data_id_; // JniFrontend.loadTableData
  jmethodID set_catalog_initialized_id_; // JniFrontend.setCatalogInitialized
  jmethodID fe_ctor_;
};

}

#endif
