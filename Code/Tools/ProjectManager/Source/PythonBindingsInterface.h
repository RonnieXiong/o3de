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

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/Outcome/Outcome.h>

#include <EngineInfo.h>
#include <GemCatalog/GemInfo.h>
#include <ProjectInfo.h>
#include <ProjectTemplateInfo.h>

namespace O3DE::ProjectManager
{
    //! Interface used to interact with the o3de cli python functions
    class IPythonBindings
    {
    public:
        AZ_RTTI(O3DE::ProjectManager::IPythonBindings, "{C2B72CA4-56A9-4601-A584-3B40E83AA17C}");
        AZ_DISABLE_COPY_MOVE(IPythonBindings);

        IPythonBindings() = default;
        virtual ~IPythonBindings() = default;


        // Engine

        /**
         * Get info about the engine 
         * @return an outcome with EngineInfo on success
         */
        virtual AZ::Outcome<EngineInfo> GetEngineInfo() = 0;

        /**
         * Set info about the engine 
         * @param engineInfo an EngineInfo object 
         */
        virtual bool SetEngineInfo(const EngineInfo& engineInfo) = 0;


        // Gems

        /**
         * Get info about a Gem 
         * @param path the absolute path to the Gem 
         * @return an outcome with GemInfo on success 
         */
        virtual AZ::Outcome<GemInfo> GetGem(const QString& path) = 0;

        /**
         * Get info about all known Gems
         * @return an outcome with GemInfos on success 
         */
        virtual AZ::Outcome<QVector<GemInfo>> GetGems() = 0;


        // Projects 

        /**
         * Create a project 
         * @param projectTemplate the project template to use 
         * @param projectInfo the project info to use 
         * @return an outcome with ProjectInfo on success 
         */
        virtual AZ::Outcome<ProjectInfo> CreateProject(const ProjectTemplateInfo& projectTemplate, const ProjectInfo& projectInfo) = 0;
        
        /**
         * Get info about a project 
         * @param path the absolute path to the project 
         * @return an outcome with ProjectInfo on success 
         */
        virtual AZ::Outcome<ProjectInfo> GetProject(const QString& path) = 0;

        /**
         * Get info about all known projects
         * @return an outcome with ProjectInfos on success 
         */
        virtual AZ::Outcome<QVector<ProjectInfo>> GetProjects() = 0;

        /**
         * Update a project
         * @param projectInfo the info to use to update the project 
         * @return true on success, false on failure
         */
        virtual bool UpdateProject(const ProjectInfo& projectInfo) = 0;


        // Project Templates

        /**
         * Get info about all known project templates
         * @return an outcome with ProjectTemplateInfos on success 
         */
        virtual AZ::Outcome<QVector<ProjectTemplateInfo>> GetProjectTemplates() = 0;
    };

    using PythonBindingsInterface = AZ::Interface<IPythonBindings>;
} // namespace O3DE::ProjectManager