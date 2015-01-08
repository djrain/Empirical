#ifndef EMP_INSTRUCTION_LIB_H
#define EMP_INSTRUCTION_LIB_H

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  The InstLib class maintains a library of all instructions available to a particular type
//  of virtual CPU, including the functions associated with them, their costs, etc.
//
//  Note: This class is templated on a HARDWARE_TYPE and an INST_TYPE, and thus can be flexible.
//  * HARDWARE_TYPE& is used for the first input for all instruction functions
//  * INST_TYPE must have a working GetID() to transform it into a unique integer.
//

#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../tools/assert.h"
#include "../tools/errors.h"
#include "../tools/functions.h"
#include "../tools/string_utils.h"

namespace emp {

  // The InstDefinition struct provides the core definition for a possible instruction, linking
  // a name to its description and associate function call.

  template <class HARDWARE_TYPE> struct InstDefinition {
    std::string desc;
    std::function<bool(HARDWARE_TYPE&)> call;    
  };


  // The InstInfo struct provides detailed information for an instruction implementation
  // active in this instruction set.

  template <typename INST_TYPE> struct InstInfo {
    // User-specified data for each instruction
    std::string name;    // Name of this instruction
    std::string desc;    // Description of this instruction
    int arg_value;       // If used as an argument, what is its value?

    // Auto-generated by InstLib
    char short_name;     // Single character representation of this instruction
    int id;              // Unique ID indicating position of this instruction in the set.
    INST_TYPE prototype; // example of this instruction to be handed out.

    // Arguments
    int cycle_cost;      // CPU Cycle Cost to execute this instruction (default = 1)
    double stability;    // Probability of this cite resisting a mutation (default = 0.0)
    double weight;       // Relative probability of mutating to this instruction (default = 1.0)

    InstInfo(const std::string & _name, const std::string & _desc, int _arg, char _sname, int _id,
             int _cycle_cost, double _stability, double _weight)
      : name(_name), desc(_desc), arg_value(_arg), short_name(_sname), id(_id)
      , prototype(_id, _arg+1, _cycle_cost != 1)
      , cycle_cost(_cycle_cost), stability(_stability), weight(_weight)
    { ; }
  };

  const char inst_char_chart[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 
                                   'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 
                                   'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                   'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 
                                   'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', '!', '@', '$', '%', '^', '&', '*', '_', '=', '-',
                                   '+' };

  template <typename HARDWARE_TYPE, typename INST_TYPE> class InstLib {
  private:
    // Information of the Instructions associated with this InstLib
    // Instruction function pointers are separated out for improved (?) cache performance.
    std::vector< std::function<bool(HARDWARE_TYPE&)> > inst_calls;
    std::vector<InstInfo<INST_TYPE> > inst_info;

    std::map<std::string, int> name_map;
    std::map<char, int> short_name_map;

  public:
    InstLib() { ; }
    ~InstLib() { ; }

    int GetSize() const { return (int) inst_info.size(); }

    // Indexing into an InstLib (by id, name, or symbol) will return  an example instruction 
    const INST_TYPE & operator[](int index) {
      emp_assert(index >= 0 && index < (int) inst_info.size());
      return inst_info[index].prototype;
    }
    const INST_TYPE & operator[](std::string name) {
      if (name_map.find(name) == name_map.end()) {
        std::stringstream ss;
        ss << "Trying to access unknown instruction '" << name << "'.  Using default.";
        NotifyError(ss.str());
      }
      return inst_info[name_map[name]].prototype;
    }
    const INST_TYPE & operator[](char symbol) {
      if (short_name_map.find(symbol) == short_name_map.end()) {
        std::stringstream ss;
        ss << "No known instruction associated with symbol '" << symbol << "'.  Using default.";
        NotifyError(ss.str());
      }
      return inst_info[short_name_map[symbol]].prototype;
    }

    inline bool RunInst(HARDWARE_TYPE & hw, int inst_id) const {
      emp_assert(inst_id >= 0 && inst_id < inst_calls.size());
      return inst_calls[inst_id](hw);
    }

    // Add a new instruction to this library.
    InstLib & Add(const std::string & name, const std::string & desc,
                  std::function<bool(HARDWARE_TYPE&)> call, int arg=-1,
                  int cycle_cost=1, double stability=0.0, double weight=1.0) {
      // Make sure we don't have another instruction by this exact name already.
      if (name_map.find(name) != name_map.end()) {
        std::stringstream ss;
        ss << "Adding duplicate instruction name '" << name << "' to instruction library.  Ignoring.";
        NotifyWarning(ss.str());
        return *this;
      }

      // Generate ID information for this new instruction.
      const int next_id = (int) inst_info.size();  // The ID number of this new instruction.
      const int char_id = std::min(next_id, 72);   // We only have 72 chars, so additional are "+"
      const char next_char = inst_char_chart[char_id];

      // Save this function call separately from everything else for fast lookup.
      inst_calls.push_back(call);

      // Save all of the other information
      inst_info.push_back( InstInfo<INST_TYPE>(name, desc, arg, next_char, next_id,
                                               cycle_cost, stability, weight) );

      // Make sure we can look up this instruction quickly by name or char ID.
      name_map[name] = next_id;
      if (next_id == char_id) short_name_map[next_char] = next_id;

      return *this;
    }


    // Retrieve information about each instruction.
    const std::string & GetName(const INST_TYPE & inst) const { return inst_info[inst.GetID()].name; }
    char GetShortName(const INST_TYPE & inst) const { return inst_info[inst.GetID()].short_name; }
    int GetCycleCost(const INST_TYPE & inst) const { return inst_info[inst.GetID()].cycle_cost; }
    int GetID(const INST_TYPE & inst) const { return inst_info[inst.GetID()].id; }

    // Convert an INST_TYPE into a single character (only works perfectly if inst lib size < 72)
    char AsChar(const INST_TYPE & inst) const { return inst_info[inst.GetID()].short_name; }
    
    //Convert an instruction vector into a series of characters.
    std::string AsString(const std::vector<INST_TYPE> & inst_vector) const {
      const int vector_size= inst_vector.GetSize();
      std::string out_string(vector_size);
      for (int i = 0; i < vector_size; i++) {
        out_string[i] = ToChar(inst_vector[i]);
      }
      // @CAO Should we do something here to facilitate move sematics?
      return out_string;
    }

    // The following function will load a specified instruction into this instruction library.
    //
    // The incoming string should look like:
    //  inst_name:spec_id:custom_name arg1=value arg2=value ...
    //
    // The instruction name has up to three components:
    //   inst_name is the built-in name for the instruction (i.e., "Nop", "Inc", or "Divide")
    //   spec_id is the component number this instruction should be associated with if it
    //       is used as an argument (usually just nops are treated this way, such as Nop:3)
    //   custom_name is anything the user would like it to be; it allows otherwise identical
    //       instructions to be distinct and treated as such.
    //
    // For example, "Nop:3" is a No-operation instruction that is associated with component 3
    // when used as an argument.  Instructions can have additional information placed after a
    // second ':' that is ignored, but attached to the name.  So "Inc::MyFavoriteInst" will
    // behave the same as "Inc".  Likewise "Nop:3:v2" will behave the same as "Nop:3".
    //
    // Other arguments in an instruction definition specify additional details for how this
    // instruction should behave in non-standard ways.  They include:
    //   cycle_cost - The number of CPU cycles that must be spent to execute this instruction.
    //       (type=int; range=1+; default=1)
    //   stability - The additional probability of this instruction "resisting" a mutation.
    //       (type=double; range=0.0-1.0; default = 0.0)
    //   weight - The relative probability of mutating to this instruction during a mutation.
    //       (type=double; range=0.0+; default = 1.0)
    // 

    bool LoadInst(std::string inst_info)
    {
      // Determine the instruction name.
      compress_whitespace(inst_info);
      std::string full_name = string_pop_word(inst_info);  // Full name of instruction  eg: Nop:3:v2
      std::string name_info = full_name;                   // Extra info at end of name eg: 3:v2
      std::string name_base = string_pop(name_info, ':');  // Base name of instruction  eg: Nop
      std::string name_spec = string_get(name_info, ':');  // First info after ':'      eg: 3
      std::string name_final = full_name;                  // Name inst should be stored under
      int mod_id = name_spec.size() ? std::stoi(name_spec) : -1;

      // Set all of the arguments to their defaults.
      int cycle_cost = 1;
      double stability = 0.0;
      double weight = 1.0;

      // Collect additional arguments.
      while(inst_info.size() > 0) {
        std::string arg_info = string_pop_word(inst_info);  // Value assigned to (rhs)
        std::string arg_name = string_pop(arg_info, '=');   // Variable name.
        
        if (arg_name == "cycle_cost") {
          cycle_cost = std::stoi(arg_info);
          if (cycle_cost < 1) {
            std::stringstream ss;
            ss << "Trying to set '" << full_name << "' cycle_cost to " << cycle_cost
               << ". Using minimum of 1 instead.";
            NotifyError(ss.str());
            cycle_cost = 1;
          }
        }
        else if (arg_name == "name") {
          if (arg_info.size() == 0) {
            std::stringstream ss;
            ss << "Trying to set '" << full_name << "' to have no name.  Ignoring.";
            NotifyError(ss.str());
          }
          else {
            name_final = arg_info;
          }
        }
        else if (arg_name == "stability") {
          stability = std::stoi(arg_info);
          if (stability < 0.0 || stability > 1.0) {
            std::stringstream ss;
            ss << "Trying to set '" << full_name << "' stability to " << stability;
            stability = to_range(stability, 0.0, 1.0);
            ss << ". Using extreme of " << stability << " instead.";
            NotifyError(ss.str());
          }
        }
        else if (arg_name == "weight") {
          weight = std::stod(arg_info);
          if (weight < 0.0) {
            std::stringstream ss;
            ss << "Trying to set '" << full_name << "' cycle_cost to " << weight
               << ". Using minimum of 0 instead.";
            NotifyError(ss.str());
            weight = 0.0;
          }
        }
        else {
          std::stringstream ss;
          ss << "Unknown argument '" << arg_name << "'.  Ignoring.";
          NotifyError(ss.str());
        }
      }

      auto inst_defs = HARDWARE_TYPE::GetInstDefs();
      if (inst_defs.find(name_base) == inst_defs.end()) {
        std::stringstream ss;
        ss << "Failed to find instruction '" << name_base << "'.  Ignoring.";
        NotifyError(ss.str());
        
        return false;
      }
    
      auto cur_def = inst_defs[name_base];

      Add(name_final, cur_def.desc, cur_def.call, mod_id, cycle_cost, stability, weight);

      return true;
    }

    void LoadDefaults() {
      auto default_insts = HARDWARE_TYPE::GetDefaultInstructions();
      for (const std::string & inst_name : default_insts) {
        LoadInst(inst_name);
      }
    }
    

  };
};

#endif
