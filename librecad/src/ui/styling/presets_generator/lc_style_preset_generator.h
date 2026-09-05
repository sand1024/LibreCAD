/*******************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2026 LibreCAD.org
 * Copyright (C) 2026 sand1024
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef LC_StylePresetGenerator_H
#define LC_StylePresetGenerator_H
#include <QString>
#include <memory>

class LC_PaletteRepository;
class LC_FusionSkinsRepository;
class LC_IconsStyleRepository;
class LC_TypographyRepository;
class LC_MetricsRepository;
class LC_UIStyleManager;

class LC_StylePresetGenerator {
public:
    explicit LC_StylePresetGenerator(LC_UIStyleManager* styleManager);
    explicit LC_StylePresetGenerator(const QString& baseConfigDir);
    LC_StylePresetGenerator(const QString& palettesDir,
                            const QString& skinsDir,
                            const QString& iconsDir,
                            const QString& typographyDir,
                            const QString& metricsDir);
    ~LC_StylePresetGenerator();

    int generatePalettes(int count);
    int generateSkins(int count);
    int generateIconStyles(int count, bool shortWheel = true);
    int generateTypography(int count);
    int generateMetrics(int count);

private:
    std::unique_ptr<LC_PaletteRepository>     m_paletteRepo;
    std::unique_ptr<LC_FusionSkinsRepository> m_skinsRepo;
    std::unique_ptr<LC_IconsStyleRepository>  m_iconsRepo;
    std::unique_ptr<LC_TypographyRepository>  m_typographyRepo;
    std::unique_ptr<LC_MetricsRepository>     m_metricsRepo;
};

#endif
