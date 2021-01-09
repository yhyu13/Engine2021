//
// Created by robin on 31.07.2019.
//

#include "Map.h"
#include "../objects/Color.hpp"
#include "../objects/Vector2.hpp"

/*!
 * When errors have happened before the map starts parsing, just keep the statuses
 * @param status The status
 * @param description Description of the status
 */
tson::Map::Map(tson::Map::ParseStatus status, std::string description) : m_status {status}, m_statusMessage { std::move(description) }
{

}

/*!
 * Parses a json of a Tiled map.
 * @param json A json object with the format of Map
 * @return true if all mandatory fields was found. false otherwise.
 */
tson::Map::Map(const nlohmann::json &json)
{
    parse(json);
}

/*!
 * Parses a json of a Tiled map.
 * @param json A json object with the format of Map
 * @return true if all mandatory fields was found. false otherwise.
 */
bool tson::Map::parse(const nlohmann::json &json)
{
    bool allFound = true;
    if(json.count("backgroundcolor") > 0) m_backgroundColor = Colori(json["backgroundcolor"].get<std::string>()); //Optional
    if(json.count("width") > 0 && json.count("height") > 0 )
        m_size = {json["width"].get<int>(), json["height"].get<int>()}; else allFound = false;
    if(json.count("hexsidelength") > 0) m_hexsideLength = json["hexsidelength"].get<int>();         //Optional
    if(json.count("infinite") > 0) m_isInfinite = json["infinite"].get<bool>();                     //Optional
    if(json.count("nextlayerid") > 0) m_nextLayerId = json["nextlayerid"].get<int>();               //Optional
    if(json.count("nextobjectid") > 0) m_nextObjectId = json["nextobjectid"].get<int>(); else allFound = false;
    if(json.count("orientation") > 0) m_orientation = json["orientation"].get<std::string>(); else allFound = false;
    if(json.count("renderorder") > 0) m_renderOrder = json["renderorder"].get<std::string>();       //Optional
    if(json.count("staggeraxis") > 0) m_staggerAxis = json["staggeraxis"].get<std::string>();       //Optional
    if(json.count("staggerindex") > 0) m_staggerIndex = json["staggerindex"].get<std::string>();    //Optional
    if(json.count("tiledversion") > 0) m_tiledVersion = json["tiledversion"].get<std::string>(); else allFound = false;
    if(json.count("tilewidth") > 0 && json.count("tileheight") > 0 )
        m_tileSize = {json["tilewidth"].get<int>(), json["tileheight"].get<int>()}; else allFound = false;
    if(json.count("type") > 0) m_type = json["type"].get<std::string>();                            //Optional
    if(json.count("version") > 0) m_version = json["version"].get<int>(); else allFound = false;

    //More advanced data
    if(json.count("layers") > 0 && json["layers"].is_array())
        std::for_each(json["layers"].begin(), json["layers"].end(), [&](const nlohmann::json &item) { m_layers.emplace_back(item); });
    if(json.count("tilesets") > 0 && json["tilesets"].is_array())
        std::for_each(json["tilesets"].begin(), json["tilesets"].end(), [&](const nlohmann::json &item) { m_tilesets.emplace_back(item); });
    if(json.count("properties") > 0 && json["properties"].is_array())
        std::for_each(json["properties"].begin(), json["properties"].end(), [&](const nlohmann::json &item) { m_properties.add(item); });


    processData();

    return allFound;
}

/*!
 * Processes the parsed data and uses the data to create helpful objects, like tile maps.
 */
void tson::Map::processData()
{
    m_tileMap.clear();
    for(auto &tileset : m_tilesets)
    {
        std::for_each(tileset.getTiles().begin(), tileset.getTiles().end(), [&](tson::Tile &tile) { m_tileMap[tile.getId()] = &tile; });
    }
    std::for_each(m_layers.begin(), m_layers.end(), [&](tson::Layer &layer)
                                                    {
                                                        layer.assignTileMap(m_tileMap);
                                                        layer.createTileData(m_size, m_isInfinite);
                                                    });
}

/*!
 * 'backgroundcolor': Color created from a hex-formatted color string (#RRGGBB or #AARRGGBB) (optional)
 * @return string as color
 */
const tson::Colori &tson::Map::getBackgroundColor() const
{
    return m_backgroundColor;
}

/*!
 * 'width' and 'height' of a Tiled map
 * @return
 */
const tson::Vector2<int> &tson::Map::getSize() const
{
    return m_size;
}

/*!
 * 'hexsidelength': Length of the side of a hex tile in pixels
 * @return
 */
int tson::Map::getHexsideLength() const
{
    return m_hexsideLength;
}

/*!
 * 'infinite': Whether the map has infinite dimensions
 * @return
 */
bool tson::Map::isInfinite() const
{
    return m_isInfinite;
}

/*!
 * 'nextlayerid': Auto-increments for each layer
 * @return
 */
int tson::Map::getNextLayerId() const
{
    return m_nextLayerId;
}

/*!
 * 'nextobjectid': Auto-increments for each placed object
 * @return
 */
int tson::Map::getNextObjectId() const
{
    return m_nextObjectId;
}

/*!
 * 'orientation': orthogonal, isometric, staggered or hexagonal
 * @return
 */
const std::string &tson::Map::getOrientation() const
{
    return m_orientation;
}

/*!
 * 'renderorder': Rendering direction (orthogonal maps only)
 * @return
 */
const std::string &tson::Map::getRenderOrder() const
{
    return m_renderOrder;
}

/*!
 * 'staggeraxis': x or y (staggered / hexagonal maps only)
 * @return
 */
const std::string &tson::Map::getStaggerAxis() const
{
    return m_staggerAxis;
}

/*!
 * 'staggerindex': odd or even (staggered / hexagonal maps only)
 * @return
 */
const std::string &tson::Map::getStaggerIndex() const
{
    return m_staggerIndex;
}

/*!
 * 'tiledversion': The Tiled version used to save the file
 * @return
 */
const std::string &tson::Map::getTiledVersion() const
{
    return m_tiledVersion;
}

/*!
 * 'tilewidth': and 'tileheight' of a map
 * @return
 */
const tson::Vector2<int> &tson::Map::getTileSize() const
{
    return m_tileSize;
}

/*!
 * 'type': map (since 1.0)
 * @return
 */
const std::string &tson::Map::getType() const
{
    return m_type;
}

/*!
 * 'version': The JSON format version
 * @return
 */
int tson::Map::getVersion() const
{
    return m_version;
}

/*!
 * 'layers': Array of layers. group on
 * @return
 */
std::vector<tson::Layer> &tson::Map::getLayers()
{
    return m_layers;
}

/*!
 * 'properties': A list of properties (name, value, type).
 * @return
 */
tson::PropertyCollection &tson::Map::getProperties()
{
    return m_properties;
}

/*!
 * 'tilesets': Array of Tilesets
 * @return
 */
std::vector<tson::Tileset> &tson::Map::getTilesets()
{
    return m_tilesets;
}

tson::Layer *tson::Map::getLayer(const std::string &name)
{
    auto result = std::find_if(m_layers.begin(), m_layers.end(), [&](const tson::Layer &item) { return item.getName() == name; });
    if(result == m_layers.end())
        return nullptr;

    return &result.operator*();
}

tson::Tileset *tson::Map::getTileset(const std::string &name)
{
    auto result = std::find_if(m_tilesets.begin(), m_tilesets.end(), [&](const tson::Tileset &item) {return item.getName() == name; });
    if(result == m_tilesets.end())
        return nullptr;

    return &result.operator*();
}

/*!
 * Shortcut for getting a property object. Alternative to getProperties().getProperty("<name>");
 * @param name Name of the property
 * @return
 */
tson::Property *tson::Map::getProp(const std::string &name)
{
    if(m_properties.hasProperty(name))
        return m_properties.getProperty(name);
    return nullptr;
}

tson::Map::ParseStatus tson::Map::getStatus() const
{
    return m_status;
}

const std::string &tson::Map::getStatusMessage() const
{
    return m_statusMessage;
}

/*!
 * Get a tile map with pointers to every existing tile.
 * @return
 */
const std::map<int, tson::Tile *> &tson::Map::getTileMap() const
{
    return m_tileMap;
}


