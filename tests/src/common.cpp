#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "logger.h"
#include "objects/base/instance.h"
#include "objects/service/script/scriptcontext.h"
#include "objects/service/script/serverscriptservice.h"
#include "objects/service/workspace.h"
#include "testcommon.h"

std::shared_ptr<DataModel> gTestModel;
std::stringstream testLogOutput;

class commonTestListener : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const&) override {
        // TODO: Make physicsInit optional in headless environments
        physicsInit();
        
        gTestModel = DataModel::New();
        gTestModel->Init(true);
        Logger::initTest(&testLogOutput);
    }
    
    void testRunEnded(Catch::TestRunStats const&) override {
        gTestModel = nullptr;
        physicsDeinit();
        Logger::initTest(nullptr);
    }
    
    void testCasePartialStarting(const Catch::TestCaseInfo &testInfo, uint64_t partNumber) override {
        // Clear the log output prior to each test
        testLogOutput.str("");
    }

    void testCasePartialEnded(const Catch::TestCaseStats &testCaseStats, uint64_t partNumber) override {    
        auto ctx = gTestModel->GetService<ScriptContext>();
        ctx->DebugClearSleepingThreads();
        
        // Clean up remaining scripts from ServerScriptService        
        for (auto& obj : gTestModel->GetService<ServerScriptService>()->GetChildren()) {
            obj->Destroy();
        }
        
        // Also clear workspace
        for (auto& obj : gTestModel->GetService<Workspace>()->GetChildren()) {
            obj->Destroy();
        }
    }
};

CATCH_REGISTER_LISTENER(commonTestListener)