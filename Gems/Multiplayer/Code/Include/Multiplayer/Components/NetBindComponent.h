/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/Entity.h>
#include <AzCore/Math/Aabb.h>
#include <AzCore/std/containers/map.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzNetworking/Serialization/ISerializer.h>
#include <AzNetworking/ConnectionLayer/IConnection.h>
#include <Multiplayer/NetworkEntity/EntityReplication/ReplicationRecord.h>
#include <Multiplayer/NetworkEntity/NetworkEntityHandle.h>
#include <Multiplayer/NetworkInput/IMultiplayerComponentInput.h>
#include <Multiplayer/NetworkTime/INetworkTime.h>
#include <Multiplayer/MultiplayerTypes.h>
#include <AzCore/EBus/Event.h>

namespace Multiplayer
{
    class NetworkInput;
    class ReplicationRecord;
    class MultiplayerComponent;

    using EntityStopEvent = AZ::Event<const ConstNetworkEntityHandle&>;
    using EntityDirtiedEvent = AZ::Event<>;
    using EntityMigrationStartEvent = AZ::Event<ClientInputId>;
    using EntityMigrationEndEvent = AZ::Event<>;
    using EntityServerMigrationEvent = AZ::Event<const ConstNetworkEntityHandle&, HostId, AzNetworking::ConnectionId>;

    //! @class NetBindComponent
    //! @brief Component that provides net-binding to a networked entity.
    class NetBindComponent final
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(NetBindComponent, "{DAA076B3-1A1C-4FEF-8583-1DF696971604}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        NetBindComponent();
        ~NetBindComponent() override = default;

        //! AZ::Component overrides.
        //! @{
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        //! @}

        NetEntityRole GetNetEntityRole() const;
        bool IsAuthority() const;
        bool IsAutonomous() const;
        bool IsServer() const;
        bool IsClient() const;
        bool HasController() const;
        NetEntityId GetNetEntityId() const;
        const PrefabEntityId& GetPrefabEntityId() const;
        ConstNetworkEntityHandle GetEntityHandle() const;
        NetworkEntityHandle GetEntityHandle();

        void SetOwningConnectionId(AzNetworking::ConnectionId connectionId);
        void SetAllowAutonomy(bool value);
        MultiplayerComponentInputVector AllocateComponentInputs();
        bool IsProcessingInput() const;
        void CreateInput(NetworkInput& networkInput, float deltaTime);
        void ProcessInput(NetworkInput& networkInput, float deltaTime);

        bool HandleRpcMessage(AzNetworking::IConnection* invokingConnection, NetEntityRole remoteRole, NetworkEntityRpcMessage& message);
        bool HandlePropertyChangeMessage(AzNetworking::ISerializer& serializer, bool notifyChanges = true);

        RpcSendEvent& GetSendAuthorityToClientRpcEvent();
        RpcSendEvent& GetSendAuthorityToAutonomousRpcEvent();
        RpcSendEvent& GetSendServerToAuthorityRpcEvent();
        RpcSendEvent& GetSendAutonomousToAuthorityRpcEvent();

        const ReplicationRecord& GetPredictableRecord() const;

        void MarkDirty();
        void NotifyLocalChanges();
        void NotifyMigrationStart(ClientInputId migratedInputId);
        void NotifyMigrationEnd();
        void NotifyServerMigration(HostId hostId, AzNetworking::ConnectionId connectionId);

        void AddEntityStopEventHandler(EntityStopEvent::Handler& eventHandler);
        void AddEntityDirtiedEventHandler(EntityDirtiedEvent::Handler& eventHandler);
        void AddEntityMigrationStartEventHandler(EntityMigrationStartEvent::Handler& eventHandler);
        void AddEntityMigrationEndEventHandler(EntityMigrationEndEvent::Handler& eventHandler);
        void AddEntityServerMigrationEventHandler(EntityServerMigrationEvent::Handler& eventHandler);

        bool SerializeEntityCorrection(AzNetworking::ISerializer& serializer);

        bool SerializeStateDeltaMessage(ReplicationRecord& replicationRecord, AzNetworking::ISerializer& serializer);
        void NotifyStateDeltaChanges(ReplicationRecord& replicationRecord);

        void FillReplicationRecord(ReplicationRecord& replicationRecord) const;
        void FillTotalReplicationRecord(ReplicationRecord& replicationRecord) const;

    private:
        void PreInit(AZ::Entity* entity, const PrefabEntityId& prefabEntityId, NetEntityId netEntityId, NetEntityRole netEntityRole);

        void ConstructControllers();
        void DestructControllers();
        void ActivateControllers(EntityIsMigrating entityIsMigrating);
        void DeactivateControllers(EntityIsMigrating entityIsMigrating);

        void OnEntityStateEvent(AZ::Entity::State oldState, AZ::Entity::State newState);

        void NetworkAttach();

        void HandleMarkedDirty();
        void HandleLocalServerRpcMessage(NetworkEntityRpcMessage& message);

        void DetermineInputOrdering();

        void StopEntity();

        ReplicationRecord m_currentRecord = NetEntityRole::InvalidRole;
        ReplicationRecord m_totalRecord = NetEntityRole::InvalidRole;
        ReplicationRecord m_predictableRecord = NetEntityRole::Autonomous;
        ReplicationRecord m_localNotificationRecord = NetEntityRole::InvalidRole;
        PrefabEntityId    m_prefabEntityId;
        AZStd::unordered_map<NetComponentId, MultiplayerComponent*> m_multiplayerComponentMap;
        AZStd::vector<MultiplayerComponent*> m_multiplayerSerializationComponentVector;
        AZStd::vector<MultiplayerComponent*> m_multiplayerInputComponentVector;

        RpcSendEvent m_sendAuthorityToClientRpcEvent;
        RpcSendEvent m_sendAuthorityToAutonomousRpcEvent;
        RpcSendEvent m_sendServertoAuthorityRpcEvent;
        RpcSendEvent m_sendAutonomousToAuthorityRpcEvent;

        EntityStopEvent       m_entityStopEvent;
        EntityDirtiedEvent    m_dirtiedEvent;
        EntityMigrationStartEvent  m_entityMigrationStartEvent;
        EntityMigrationEndEvent    m_entityMigrationEndEvent;
        EntityServerMigrationEvent m_entityServerMigrationEvent;
        AZ::Event<>           m_onRemove;
        RpcSendEvent::Handler m_handleLocalServerRpcMessageEventHandle;
        AZ::Event<>::Handler  m_handleMarkedDirty;
        AZ::Event<>::Handler  m_handleNotifyChanges;
        AZ::Entity::EntityStateEvent::Handler m_handleEntityStateEvent;

        NetworkEntityHandle   m_netEntityHandle;
        NetEntityRole         m_netEntityRole   = NetEntityRole::InvalidRole;
        NetEntityId           m_netEntityId     = InvalidNetEntityId;

        bool                  m_isProcessingInput    = false;
        bool                  m_isMigrationDataValid = false;
        bool                  m_needsToBeStopped     = false;
        bool                  m_allowAutonomy        = false; // Set to true for the hosts controlled entity

        friend class NetworkEntityManager;
        friend class EntityReplicationManager;
    };

    bool NetworkRoleHasController(NetEntityRole networkRole);
}