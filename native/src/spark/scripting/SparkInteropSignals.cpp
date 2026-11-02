#include "spark/scripting/SparkInterop.h"

#include "spark/ecs/GameComponent.hpp"
#include "spark/ecs/GameObject.hpp"
#include "spark/ecs/Signal.hpp"

extern "C" {

void spark_object_emit_signal(
        SparkGameObject* object,
        const SparkSignalId id,
        const SparkSignalPayload* payload,
        SparkGameComponent* sender) {
    if (object == nullptr) {
        return;
    }
    auto* obj = reinterpret_cast<Spark::GameObject*>(object);
    Spark::SignalPayload cppPayload{};
    if (payload != nullptr) {
        cppPayload.ptr = payload->ptr;
        cppPayload.a = payload->a;
        cppPayload.b = payload->b;
    }
    auto* cppSender = reinterpret_cast<Spark::GameComponent*>(sender);
    obj->EmitSignal(static_cast<Spark::SignalId>(static_cast<std::uint32_t>(id)), cppPayload, cppSender);
}

}  // extern "C"
