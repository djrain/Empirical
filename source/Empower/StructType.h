/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2018
 *
 *  @file  StructType.h
 *  @brief StructType maps variables to a MemoryImage; Struct is an instance of StructType
 * 
 *  @todo Immediately before setting a StructType to active, we can optimize variable ordering.
 */

#ifndef EMP_EMPOWER_STRUCT_TYPE_H
#define EMP_EMPOWER_STRUCT_TYPE_H

#include <unordered_map>

#include "../base/assert.h"
#include "../base/Ptr.h"
#include "../base/vector.h"

#include "MemoryImage.h"
#include "TypeManager.h"
#include "VarInfo.h"

namespace emp {

  class StructType {
  private:
    emp::vector<VarInfo> vars;   ///< Member variables declared in this structure.
    std::unordered_map<std::string, size_t> name_map;   ///< Lookup table for var names.
    TypeManager & type_manager;  ///< TypeManager to track type information in this structure.
    size_t num_bytes;            ///< How big are structs of this type?
    mutable bool active;         ///< Have Structs of this type been built?  If so, do not extend.

  public:
    StructType(TypeManager & _tmanager) : type_manager(_tmanager), num_bytes(0), active(false) { ; }
    ~StructType() { ; }

    size_t GetSize() const { return num_bytes; }  ///< How many bytes in Structs of this type?
    bool IsActive() const { return active; }      ///< Have any Structs of this type been built?

    /// Look up the ID of a variable based on its name.
    size_t GetID(const std::string & name) {
      emp_assert(!Has(name_map, name));
      return name_map[name];
    }

    /// And a new member variable to structs of this type.
    template <typename T>
    void AddMemberVar(const std::string & name) {
      emp_assert(active == false, "Cannot add member variables to an instantiated struct!");
      emp_assert(!Has(name_map, name), "All member vars in emp::Struct must be unique!");
      const Type & type = type_manager.GetType<T>();
      name_map[name] = vars.size();
      vars.emplace_back(type, name, num_bytes);
      num_bytes += type.GetSize();
    }

    /// Construct a memory image using all default constructors.
    void DefaultConstruct(MemoryImage & memory) const {
      memory.resize(num_bytes);
      for (const VarInfo & vinfo : vars) vinfo.DefaultConstruct(memory);
      active = true;
    }

    /// Construct a memory image using another memory image.
    void CopyConstruct(const MemoryImage & from_memory, MemoryImage & to_memory) const {
      to_memory.resize(num_bytes);
      for (const VarInfo & vinfo : vars) vinfo.CopyConstruct(from_memory, to_memory);
      active = true;
    }
  };

}

#endif