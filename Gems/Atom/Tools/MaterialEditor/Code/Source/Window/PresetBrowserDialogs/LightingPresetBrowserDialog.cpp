/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Atom/Feature/Utils/LightingPreset.h>
#include <Atom/Viewport/MaterialViewportRequestBus.h>
#include <AtomToolsFramework/Util/Util.h>
#include <AzFramework/Application/Application.h>
#include <Window/PresetBrowserDialogs/LightingPresetBrowserDialog.h>

namespace MaterialEditor
{
    LightingPresetBrowserDialog::LightingPresetBrowserDialog(QWidget* parent)
        : PresetBrowserDialog(parent)
    {
        QSignalBlocker signalBlocker(this);

        setWindowTitle("Lighting Preset Browser");

        MaterialViewportRequestBus::BroadcastResult(m_initialPreset, &MaterialViewportRequestBus::Events::GetLightingPresetSelection);

        AZ::Render::LightingPresetPtrVector presets;
        MaterialViewportRequestBus::BroadcastResult(presets, &MaterialViewportRequestBus::Events::GetLightingPresets);
        AZStd::sort(presets.begin(), presets.end(), [](const auto& a, const auto& b) { return a->m_displayName < b->m_displayName; });

        QListWidgetItem* selectedItem = nullptr;
        for (const auto& preset : presets)
        {
            QImage image;
            MaterialViewportRequestBus::BroadcastResult(image, &MaterialViewportRequestBus::Events::GetLightingPresetPreview, preset);

            QListWidgetItem* item = CreateListItem(preset->m_displayName.c_str(), image);

            m_listItemToPresetMap[item] = preset;

            if (m_initialPreset == preset)
            {
                selectedItem = item;
            }
        }

        if (selectedItem)
        {
            m_ui->m_presetList->setCurrentItem(selectedItem);
            m_ui->m_presetList->scrollToItem(selectedItem);
        }
    }

    void LightingPresetBrowserDialog::SelectCurrentPreset()
    {
        auto presetItr = m_listItemToPresetMap.find(m_ui->m_presetList->currentItem());
        if (presetItr != m_listItemToPresetMap.end())
        {
            MaterialViewportRequestBus::Broadcast(&MaterialViewportRequestBus::Events::SelectLightingPreset, presetItr->second);
        }
    }

    void LightingPresetBrowserDialog::SelectInitialPreset()
    {
        MaterialViewportRequestBus::Broadcast(&MaterialViewportRequestBus::Events::SelectLightingPreset, m_initialPreset);
    }

} // namespace MaterialEditor

#include <Window/PresetBrowserDialogs/moc_LightingPresetBrowserDialog.cpp>
