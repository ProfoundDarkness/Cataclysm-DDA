#include "game.h"
#include "player.h"
#include "item_desirability.h"
#include "output.h"
#include "debug.h"
#include "item_factory.h"
#include "catacharset.h"
#include "translations.h"
#include "cata_utility.h"
#include "path_info.h"
#include "filesystem.h"
#include "input.h"
#include "worldfactory.h"
#include "itype.h"
#include "string_input_popup.h"

#include <stdlib.h>
#include <sstream>
#include <string>
#include <locale>

/* Intention is to add a new desirability property to advanced inventory.
 * The original idea was to add some color to the items like how autopickup-ed items have a mark of color on them (magenta).
 * Turns out adding a splash of color is actually kind of difficult since there are a variety of potential marks desired and I would
 * need a largely exhaustive list of colors that can happen which would also block colors from being used in the future...
 *
 * So falling back on the simpler idea, adding a symbol (number) to the start of the item.  This involves adding an additional column
 * to advanced inventory, the symbol can only ever be one character.
 */

item_desirability &get_item_desirability()
{
    static item_desirability single_instance = item_desirability();
    return single_instance;
}

void item_desirability::clear()
{
    loaded = false;
    map_interest.clear();
}

char item_desirability::get(const std::string &sItemName)
{
    if (map_interest.find(sItemName) == map_interest.end())
        return ' ';
    return map_interest[sItemName];
}

void item_desirability::increment(const std::string &sItemName)
{
    if (map_interest.find(sItemName) == map_interest.end())
    {
        map_interest[sItemName] = '1';
        map_interest[sItemName]--;
    }

    ++map_interest[sItemName];
    if (map_interest[sItemName] > '9')
        map_interest[sItemName] = '1';
}

void item_desirability::decrement(const std::string &sItemName)
{
    if (map_interest.find(sItemName) == map_interest.end())
    {
        map_interest[sItemName] = '9';
        map_interest[sItemName]++;
    }
    --map_interest[sItemName];
    if (map_interest[sItemName] < '1')
        map_interest[sItemName] = '9';
}

void item_desirability::remove(const std::string &sItemName)
{
    if (map_interest.find(sItemName) != map_interest.end())
        map_interest.erase(sItemName);
}

bool item_desirability::save()
{
    auto savefile = FILENAMES["itemdesirability"];

        savefile = world_generator->active_world->world_path + "/" + base64_encode(g->u.name) + ".idr.json";

        const std::string player_save = world_generator->active_world->world_path + "/" + base64_encode(g->u.name) + ".sav";
        if( !file_exist( player_save ) ) {
            return true; //Character not saved yet.
        }

    return write_to_file( savefile, [&]( std::ostream &fout ) {
        JsonOut jout( fout, true );
        serialize(jout);
    }, _( "itemdesirability configuration" ) );
}

void item_desirability::load()
{
    loaded = false;
    std::string sFile = FILENAMES["itemdesirability"];
    sFile = world_generator->active_world->world_path + "/" + base64_encode(g->u.name) + ".idr.json";
    if( !read_from_file_optional( sFile, *this ) ) {
        if (save()) {
            remove_file(sFile);
        }
    }
    loaded = true;
    printf("Loaded!\n");
}

void item_desirability::serialize(JsonOut &json) const
{
    json.start_array();

    for( auto &elem : map_interest ) {
        json.start_object();

        json.member( "item", elem.first );
        json.member( "value", elem.second );

        json.end_object();
    }

    json.end_array();
}

void item_desirability::deserialize(JsonIn &jsin)
{
    map_interest.clear();

    jsin.start_array();
    while (!jsin.end_array()) {
        JsonObject jo = jsin.get_object();

        const std::string sItemName = jo.get_string("item");
        const char val = jo.get_int("value");

        map_interest[sItemName] = val;
    }
    printf("deserialized\n");
}
