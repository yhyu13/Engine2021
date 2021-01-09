//
// Created by robin on 03.08.2019.
//

#include "Frame.h"

/*!
 *
 * @param duration duration in milliseconds
 * @param tileId TileId
 */
tson::Frame::Frame(int duration, int tileId) : m_duration {duration}, m_tileId {tileId}
{

}

/*!
 * Parses frame data from json
 * @param json
 */
tson::Frame::Frame(const nlohmann::json &json)
{
    parse(json);
}

/*!
 * Parses frame data from json
 * @param json
 * @return true if all mandatory fields was found. false otherwise.
 */
bool tson::Frame::parse(const nlohmann::json &json)
{
    bool allFound = true;

    if(json.count("duration") > 0) m_duration = json["duration"].get<int>(); else allFound = false;
    if(json.count("tileid") > 0) m_tileId = json["tileid"].get<int>(); else allFound = false;

    return allFound;
}

/*!
 * 'duration': Frame duration in milliseconds
 * @return Duration in milliseconds
 */
int tson::Frame::getDuration() const
{
    return m_duration;
}

/*!
 * 'tileid': Local tile ID representing this frame
 * @return tile id
 */
int tson::Frame::getTileId() const
{
    return m_tileId;
}
