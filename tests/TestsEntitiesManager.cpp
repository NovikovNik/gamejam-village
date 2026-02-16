#include <gtest/gtest.h>
#include <functional>
#include "TestsUtils.h"
#include "../src/Entities/EntitiesManager.h"
#include "../src/Entities/Entity.h"

namespace TestsUtils {
    struct OnDestroyProviderData {
        int32_t onDestroyCalled = 0;
    };
    class OnDestroyProvider : public Provider<OnDestroyProviderData> {
    public:
        void operator()() {
            data->onDestroyCalled++;
        }
    };
}

namespace World {
    // Test entity class for testing
    class TestEntity : public Entity {
    public:
        int32_t updateCalled = 0;
        int32_t renderCalled = 0;
        int32_t onSpawnCalled = 0;
        std::function<void()> onDestroyCallback;

        bool Update(float deltaTime) override {
            updateCalled++;
            return Entity::Update(deltaTime);
        }

        void Render(float deltaTime) override {
            renderCalled++;
        }

        void OnDestroy() override {
            if (onDestroyCallback) {
                onDestroyCallback();
            }
        }

        void OnSpawn() override {
            onSpawnCalled++;
        }
    };
}

// Test fixture for EntitiesManager
class EntitiesManagerTest : public ::testing::Test {
protected:
    World::EntitiesManager manager;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(EntitiesManagerTest, SpawnEntity_CallsOnSpawn) {
    manager.Update(0.1f);

    auto* entityPtr1 = manager.SpawnEntity<World::TestEntity>();

    ASSERT_NE(entityPtr1, nullptr);
    EXPECT_EQ(entityPtr1->onSpawnCalled, 1);
    
    manager.Update(0.1f);
    EXPECT_EQ(entityPtr1->updateCalled, 1);
    EXPECT_EQ(entityPtr1->renderCalled, 0);
    EXPECT_EQ(entityPtr1->onSpawnCalled, 1);
    
    manager.Render(0.1f);
    EXPECT_EQ(entityPtr1->updateCalled, 1);
    EXPECT_EQ(entityPtr1->renderCalled, 1);
    EXPECT_EQ(entityPtr1->onSpawnCalled, 1);
    
    manager.Update(0.1f);
    EXPECT_EQ(entityPtr1->updateCalled, 2);
    EXPECT_EQ(entityPtr1->renderCalled, 1);
    EXPECT_EQ(entityPtr1->onSpawnCalled, 1);
    
    manager.Render(0.1f);
    EXPECT_EQ(entityPtr1->updateCalled, 2);
    EXPECT_EQ(entityPtr1->renderCalled, 2);
    EXPECT_EQ(entityPtr1->onSpawnCalled, 1);
    
    auto* entityPtr2 = manager.SpawnEntity<World::TestEntity>();

    ASSERT_NE(entityPtr2, nullptr);
    EXPECT_EQ(entityPtr2->onSpawnCalled, 1);

    manager.Update(0.1f);
    EXPECT_EQ(entityPtr1->updateCalled, 3);
    EXPECT_EQ(entityPtr1->renderCalled, 2);
    EXPECT_EQ(entityPtr1->onSpawnCalled, 1);
    EXPECT_EQ(entityPtr2->updateCalled, 1);
    EXPECT_EQ(entityPtr2->renderCalled, 0);
    EXPECT_EQ(entityPtr2->onSpawnCalled, 1);
    
    manager.Render(0.1f);
    EXPECT_EQ(entityPtr1->updateCalled, 3);
    EXPECT_EQ(entityPtr1->renderCalled, 3);
    EXPECT_EQ(entityPtr1->onSpawnCalled, 1);
    EXPECT_EQ(entityPtr2->updateCalled, 1);
    EXPECT_EQ(entityPtr2->renderCalled, 1);
    EXPECT_EQ(entityPtr2->onSpawnCalled, 1);

    EXPECT_EQ(manager.GetEntityCount(), 2);
}

TEST_F(EntitiesManagerTest, SpawnEntity_CallsOnDestroy) {
    manager.Update(0.1f);

    auto* entityPtr1 = manager.SpawnEntity<World::TestEntity>();
    TestsUtils::OnDestroyProvider onDestroy1Provider;
    entityPtr1->onDestroyCallback = onDestroy1Provider;
    
    ASSERT_NE(entityPtr1, nullptr);
    
    entityPtr1->Destroy();
    EXPECT_FALSE(entityPtr1->IsValid());
    EXPECT_EQ(onDestroy1Provider.data->onDestroyCalled, 0);
    
    EXPECT_EQ(manager.GetEntityCount(), 1);
    manager.Update(0.1f);
    EXPECT_EQ(onDestroy1Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(manager.GetEntityCount(), 0);

    manager.Render(0.1f);


    auto* entityPtr2 = manager.SpawnEntity<World::TestEntity>();
    TestsUtils::OnDestroyProvider onDestroy2Provider;
    entityPtr2->onDestroyCallback = onDestroy2Provider;
    
    auto* entityPtr3 = manager.SpawnEntity<World::TestEntity>();
    TestsUtils::OnDestroyProvider onDestroy3Provider;
    entityPtr3->onDestroyCallback = onDestroy3Provider;

    auto* entityPtr4 = manager.SpawnEntity<World::TestEntity>();
    TestsUtils::OnDestroyProvider onDestroy4Provider;
    entityPtr4->onDestroyCallback = onDestroy4Provider;

    auto* entityPtr5 = manager.SpawnEntity<World::TestEntity>();
    TestsUtils::OnDestroyProvider onDestroy5Provider;
    entityPtr5->onDestroyCallback = onDestroy5Provider;

    ASSERT_NE(entityPtr2, nullptr);
    ASSERT_NE(entityPtr3, nullptr);
    ASSERT_NE(entityPtr4, nullptr);
    ASSERT_NE(entityPtr5, nullptr);

    manager.Update(0.1f);
    EXPECT_EQ(entityPtr2->updateCalled, 1);
    EXPECT_EQ(entityPtr3->updateCalled, 1);
    EXPECT_EQ(entityPtr4->updateCalled, 1);
    EXPECT_EQ(entityPtr5->updateCalled, 1);
    EXPECT_EQ(manager.GetEntityCount(), 4);

    
    entityPtr2->Destroy();
    entityPtr4->Destroy();

    manager.Update(0.1f);
    EXPECT_EQ(onDestroy2Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(onDestroy3Provider.data->onDestroyCalled, 0);
    EXPECT_EQ(onDestroy4Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(onDestroy5Provider.data->onDestroyCalled, 0);
    EXPECT_EQ(entityPtr3->updateCalled, 2);
    EXPECT_EQ(entityPtr5->updateCalled, 2);
    EXPECT_EQ(manager.GetEntityCount(), 2);

    entityPtr5->Destroy();
    manager.Update(0.1f);
    EXPECT_EQ(onDestroy5Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(entityPtr3->updateCalled, 3);
    EXPECT_EQ(manager.GetEntityCount(), 1);

    entityPtr3->Destroy();
    manager.Update(0.1f);
    EXPECT_EQ(onDestroy2Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(onDestroy3Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(onDestroy4Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(onDestroy5Provider.data->onDestroyCalled, 1);
    EXPECT_EQ(manager.GetEntityCount(), 0);
}
