#include "spark/scripting/SparkInterop.h"
#include "spark/scripting/SparkInteropInternal.hpp"

#include "spark/ecs/GameComponent.hpp"
#include "spark/ecs/GameObject.hpp"
#include "spark/ecs/Signal.hpp"
#include "spark/ecs/components/script/ScriptSignalListenerComponent.hpp"

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

SparkGameComponent* spark_object_add_script_signal_listener(SparkGameObject* object) {
    if (object == nullptr) {
        return nullptr;
    }
    auto* obj = reinterpret_cast<Spark::GameObject*>(object);
    return reinterpret_cast<SparkGameComponent*>(obj->AddComponent<Spark::ScriptSignalListenerComponent>());
}

SparkGameComponent* spark_object_get_or_add_script_signal_listener(SparkGameObject* object) {
    if (object == nullptr) {
        return nullptr;
    }
    auto* obj = reinterpret_cast<Spark::GameObject*>(object);
    if (Spark::ScriptSignalListenerComponent* existing = obj->GetComponent<Spark::ScriptSignalListenerComponent>()) {
        return reinterpret_cast<SparkGameComponent*>(existing);
    }
    return spark_object_add_script_signal_listener(object);
}

void spark_script_signal_listener_set_listen_signal(
        SparkGameComponent* listener,
        const SparkSignalId id) {
    auto* self = Spark::Scripting::AsComponent<Spark::ScriptSignalListenerComponent>(
            listener,
            Spark::ComponentKind::ScriptSignalListener);
    if (self == nullptr) {
        return;
    }
    self->SetListenSignal(static_cast<Spark::SignalId>(static_cast<std::uint32_t>(id)));
}

SparkSignalId spark_script_signal_listener_get_listen_signal(const SparkGameComponent* listener) {
    const auto* self = Spark::Scripting::AsComponent<const Spark::ScriptSignalListenerComponent>(
            listener,
            Spark::ComponentKind::ScriptSignalListener);
    if (self == nullptr) {
        return SparkSignalId_None;
    }
    return static_cast<SparkSignalId>(static_cast<std::uint32_t>(self->GetListenSignal()));
}

void spark_script_signal_listener_set_callback(
        SparkGameComponent* listener,
        const SparkSignalScriptCallbackFn callback,
        void* userData) {
    auto* self = Spark::Scripting::AsComponent<Spark::ScriptSignalListenerComponent>(
            listener,
            Spark::ComponentKind::ScriptSignalListener);
    if (self == nullptr) {
        return;
    }
    self->SetScriptCallback(
            reinterpret_cast<Spark::ScriptSignalCallback>(callback),
            userData);
}

}  // extern "C"
