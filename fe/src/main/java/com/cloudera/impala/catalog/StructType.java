package com.cloudera.impala.catalog;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;

import com.cloudera.impala.common.AnalysisException;
import com.cloudera.impala.thrift.TColumnType;
import com.cloudera.impala.thrift.TStructField;
import com.cloudera.impala.thrift.TTypeNode;
import com.cloudera.impala.thrift.TTypeNodeType;
import com.google.common.base.Joiner;
import com.google.common.base.Preconditions;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;
import com.google.common.collect.Sets;

/**
 * Describes a STRUCT type. STRUCT types have a list of named struct fields.
 */
public class StructType extends Type {
  private final HashMap<String, StructField> fieldMap_ = Maps.newHashMap();
  private final ArrayList<StructField> fields_;

  public StructType(ArrayList<StructField> fields) {
    Preconditions.checkNotNull(fields);
    fields_ = fields;
    for (StructField field : fields_) {
      fieldMap_.put(field.getName().toLowerCase(), field);
    }
  }

  @Override
  public void analyze() throws AnalysisException {
    if (isAnalyzed_) return;
    Preconditions.checkNotNull(fields_);
    Preconditions.checkState(!fields_.isEmpty());
    Set<String> fieldNames = Sets.newHashSet();
    for (StructField f : fields_) {
      f.analyze();
      if (!fieldNames.add(f.getName().toLowerCase())) {
        throw new AnalysisException(
            String.format("Duplicate field name '%s' in struct '%s'",
                f.getName(), toSql()));
      }
    }
    isAnalyzed_ = true;
  }

  @Override
  public String toSql() {
    ArrayList<String> fieldsSql = Lists.newArrayList();
    for (StructField f: fields_) {
      fieldsSql.add(f.toSql());
    }
    return String.format("STRUCT<%s>", Joiner.on(",").join(fieldsSql));
  }

  public ArrayList<StructField> getFields() { return fields_; }

  public StructField getField(String fieldName) {
    return fieldMap_.get(fieldName.toLowerCase());
  }

  @Override
  public void toThrift(TColumnType container) {
    TTypeNode node = new TTypeNode();
    container.types.add(node);
    Preconditions.checkNotNull(fields_);
    Preconditions.checkNotNull(!fields_.isEmpty());
    node.setType(TTypeNodeType.STRUCT);
    node.setStruct_fields(new ArrayList<TStructField>());
    for (StructField field: fields_) {
      field.toThrift(container, node);
    }
  }

  @Override
  public boolean matchesType(Type t) { return false; }
}
