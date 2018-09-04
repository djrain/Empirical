#define CATCH_CONFIG_MAIN

#ifndef EMP_TRACK_MEM
#define EMP_TRACK_MEM
#endif

#include "../third-party/Catch/single_include/catch.hpp"

#include "Evolve/OEE.h"
#include "Evolve/World.h"
#include "Evolve/World_output.h"

TEST_CASE("OEE", "[evo]") {
    emp::Random random;
    emp::World<int> world(random, "OEEWorld");

    emp::Ptr<emp::Systematics<int, int, emp::datastruct::oee_data<int>> > sys_ptr;
    sys_ptr.New([](int org){return org;}, true, true, false);
    // world.AddSystematics(sys_ptr);
    // world.SetPopStruct_Mixed(true);

    emp::OEETracker<int, int, int> oee(sys_ptr, [](int org){return org;}, [](int org){return org;});
    oee.SetResolution(1);
    oee.SetGenerationInterval(1);
    // AddOEEFile(world, oee).SetTimingRepeat(10);
    // world.OnUpdate([&oee](size_t ud){oee.Update(ud);});
    // world.SetFitFun([](int & org){return org;});
    // world.SetMutFun([](int & org, emp::Random r){
    //     if (r.P(.0025)) {
    //         org--;
    //     } else if (r.P(.0025)) {
    //         org++;
    //     } else {
    //         return 0;
    //     }
    //     return 1;
    // });        
    
    sys_ptr->AddOrg(1, 0, 0, false);
    sys_ptr->AddOrg(2, 1, 0, false);
    sys_ptr->AddOrg(3, 2, 0, false);
    sys_ptr->PrintStatus();
    oee.Update(0);
    
    // Coalescence interval hasn't passed yet
    CHECK(oee.CoalescenceFilter().size() == 0);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 0);

    sys_ptr->SetNextParent(0);
    sys_ptr->RemoveOrgAfterRepro(2);
    sys_ptr->AddOrg(4, 2, 0, false);
    sys_ptr->PrintStatus();
    oee.Update(1);
    
    // 1 and 2 should make it through filter
    CHECK(oee.CoalescenceFilter().size() == 2);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 2);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 2);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 2);

    // If we change nothing, 4 will now pass filter
    oee.Update(2);
    CHECK(oee.CoalescenceFilter().size() == 3);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

    // If we change nothing again, change and novelty should drop to 0
    oee.Update(3);
    CHECK(oee.CoalescenceFilter().size() == 3);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

    sys_ptr->SetNextParent(0);
    sys_ptr->RemoveOrgAfterRepro(0);
    sys_ptr->AddOrg(1, 0, 0, false);
    sys_ptr->PrintStatus();

    // Replacing 1 with a copy of itself should change nothing
    oee.Update(4);
    CHECK(oee.CoalescenceFilter().size() == 3);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

    sys_ptr->SetNextParent(0);
    sys_ptr->RemoveOrgAfterRepro(0);
    sys_ptr->AddOrg(10, 0, 0, false);
    sys_ptr->PrintStatus();

    // Replacing 1 with a new descendant should change nothing at first
    // because 1 still has descendants and 10 hasn't survived filter time
    oee.Update(5);
    CHECK(oee.CoalescenceFilter().size() == 3);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 4);

    // 10 survives the filter and replaces 1 because 1 is no longer in the
    // set being filtered
    oee.Update(6);
    CHECK(oee.CoalescenceFilter().size() == 3);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);

    sys_ptr->SetNextParent(0);
    sys_ptr->RemoveOrgAfterRepro(1);
    sys_ptr->AddOrg(2, 0, 0, false);
    sys_ptr->PrintStatus();

    // Adding an independent origin of 2 should increase change but not novelty
    // (the time after this). For now, we're replacing 2, leaving it with
    // no descendants, so it should go away immediately
    oee.Update(7);
    CHECK(oee.CoalescenceFilter().size() == 2);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);

    // Now we see the bump in change
    oee.Update(8);
    CHECK(oee.CoalescenceFilter().size() == 3);
    CHECK(oee.GetDataNode("change")->GetCurrent() == 1);
    CHECK(oee.GetDataNode("novelty")->GetCurrent() == 0);
    CHECK(oee.GetDataNode("diversity")->GetCurrent() == Approx(1.5853));
    CHECK(oee.GetDataNode("complexity")->GetCurrent() == 10);



    sys_ptr.Delete();


}