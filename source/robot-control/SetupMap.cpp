#include "SetupMap.hpp"

#include <settings/ReadSettingsHelper.hpp>

#include <QtCore/QtMath>

#include <limits>

/*!
 * Constructor.
 */
SetupMap::SetupMap() :
    m_valid(false),
    m_minX(std::numeric_limits<double>::max()),
    m_minY(std::numeric_limits<double>::max()),
    m_maxX(std::numeric_limits<double>::lowest()),
    m_maxY(std::numeric_limits<double>::lowest())
{

}

/*!
 * Reads the setup from the file.
 */
void SetupMap::init(QString setupFilePath)
{
    bool successful = true;

    ReadSettingsHelper settings(setupFilePath);

    std::vector<cv::Point2f> polygon;
    settings.readVariable(QString("polygon"), polygon);
    successful = successful && (polygon.size() > 0);

    // build a graph
    if (successful) {
        qDebug() << "Successfully initialized the setup map";
        for (cv::Point2f& point : polygon) {
            m_polygon.append(PositionMeters(point));
            m_minY = qMin(m_minY, static_cast<double>(point.y));
            m_minX = qMin(m_minX, static_cast<double>(point.x));
            m_maxY = qMax(m_maxY, static_cast<double>(point.y));
            m_maxX = qMax(m_maxX, static_cast<double>(point.x));
        }
    }
    else
        qDebug() << "Could not initialize the setup map";

    // read the excluded polygons
    std::vector<std::vector<cv::Point2f>> excludedPolygons;
    settings.readVariables(QString("excludedPolygons/polygon"), excludedPolygons);
    if (excludedPolygons.size() > 0) {
        for (std::vector<cv::Point2f>& excludedPolygon : excludedPolygons) {
            WorldPolygon excludedWorldPolygon;
            for (cv::Point2f& point : excludedPolygon) {
                excludedWorldPolygon.append(PositionMeters(point));
            }
            m_excludedPolygons << excludedWorldPolygon;
        }
    }

    m_valid = successful;
}

/*!
 * Checks that the point is inside the setup.
 */
bool SetupMap::containsPoint(PositionMeters position) const
{
    bool insidePolygon = m_polygon.containsPoint(position);
    bool insideExludedPolygon = false;
    for (WorldPolygon polygon : m_excludedPolygons) {
        if (polygon.containsPoint(position)) {
            insideExludedPolygon = true;
            break;
        }
    }

    return (insidePolygon & (!insideExludedPolygon));
}
