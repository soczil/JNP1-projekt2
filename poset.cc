#include "poset.h"

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>

using namespace std;

namespace {

#ifndef NDEBUG
    const bool debug = true;
#else
    const bool debug = false;
#endif

enum ERROR {
    NONE,
    NULL_VALUE,
    WRONG_ID,
    EXISTING_VALUE,
    NONEXISTING_VALUE,
    NULL_VALUE1,
    NULL_VALUE2,
    WRONG_RELATION
};

// para <większe/równe elementy, mniejsze elementy>
typedef pair<unordered_set<int>, unordered_set<int> > poset_value;
typedef unordered_map<int, poset_value> poset;
typedef unordered_map<unsigned long, poset> posets_collection;

typedef pair<int, unordered_map<string, int> > identifiers_value;
typedef unordered_map<unsigned long, identifiers_value> identifiers_collection;

int new_poset_id = 0;

posets_collection& posets()
{
    static posets_collection posets;
    return posets;
}

identifiers_collection& identifiers()
{
    static identifiers_collection identifiers;
    return identifiers;
}

//================ funkcje do obsługi wyjścia diagnostycznego ================
string call_info_no_params(const string& func_name)
{
    string info = func_name + "()";
    return info;
}

string call_info_one_param(const string& func_name, unsigned long id)
{
    string info = func_name + "(" + to_string(id) + ")";
    return info;
}

string call_info_two_params(const string& func_name,
                            unsigned long id,
                            const string& value)
{
    string info = func_name + "(" + to_string(id) + ", \"" + value + "\")";
    return info;
}

string call_info_three_params(const string& func_name,
                              unsigned long id,
                              const string& value1,
                              const string& value2)
{
    string info = func_name + "(" + to_string(id) + ", \"" + value1 + "\", \""
                  + value2 + "\")";
    return info;
}

string poset_contains_info(unsigned long id, size_t elements)
{
    string info = "poset_size: poset " + to_string(id) + " contains "
                  + to_string(elements) + " element(s)";
    return info;
}

string element_inserted_info(unsigned long id, const string& value)
{
    string info = "poset_insert: poset " + to_string(id) + ", element \""
                  + value + "\" inserted";
    return info;
}

string element_already_exists_info(unsigned long id, const string& value)
{
    string info = "poset_insert: poset " + to_string(id) + ", element \""
                  + value + "\" already exists";
    return info;
}

string element_doesnt_exist(unsigned long id, const string& value)
{
    string info = "poset_remove: poset " + to_string(id) + ", element \""
                  + value + "\" does not exist";
    return info;
}

string element_removed(unsigned long id, const string& value)
{
    string info = "poset_remove: poset " + to_string(id) + ", element \""
                  + value + "\" removed";
    return info;
}

string invalid_value(const string& func_name, const string& value)
{
    string info = func_name + ": invalid " + value + " (NULL)";
    return info;
}

string poset_info(const string& func_name,
                  unsigned long id,
                  const string& msg)
{
    string info = func_name + ": poset " + to_string(id) + " " + msg;
    return info;
}

string poset_doesnt_exist(const string& func_name, unsigned long id)
{
    return poset_info(func_name, id, "does not exist");
}

string nonexisting_value(const string& func_name,
                         unsigned long id,
                         const string& value1,
                         const string& value2)
{
    string info = func_name + ": poset " + to_string(id) + ", element \""
                  + value1 + "\" or \"" + value2 + "\" does not exist";
    return info;
}

string relation_info(const string& func_name,
                     unsigned long id,
                     const string& value1,
                     const string& value2,
                     const string& msg)
{
    string info = func_name + ": poset " + to_string(id) + ", relation (\""
                  + value1 + "\", \"" + value2 + "\") " + msg;
    return info;
}

void poset_new_debug_info(unsigned long id)
{
    if (debug) {
        string func = "poset_new";
        cerr << call_info_no_params(func) << endl;
        cerr << poset_info(func, id, "created") << endl;
    }
}

void poset_delete_debug_info(unsigned long id, ERROR err)
{
    if (debug) {
        string func = "poset_delete";
        cerr << call_info_one_param(func, id) << endl;
        if (err == WRONG_ID) {
            cerr << poset_doesnt_exist(func, id) << endl;
        }
        else if (err == NONE) {
            cerr << poset_info(func, id, "deleted") << endl;
        }
    }
}

void poset_size_debug_info(unsigned long id, size_t elements, ERROR err)
{
    if (debug) {
        string func = "poset_size";
        cerr << call_info_one_param(func, id) << endl;
        if (err == WRONG_ID) {
            cerr << poset_doesnt_exist(func, id) << endl;
        }
        else if (err == NONE) {
            cerr << poset_contains_info(id, elements) << endl;
        }
    }
}

void poset_insert_debug_info(unsigned long id, const char* value, ERROR err)
{
    if (debug) {
        string func = "poset_insert";
        if (err == NULL_VALUE) {
            cerr << call_info_two_params(func, id, "NULL") << endl;
            cerr << invalid_value(func, "value") << endl;
        }
        else {
            cerr << call_info_two_params(func, id, value) << endl;
            switch (err) {
                case NONE:
                    cerr << element_inserted_info(id, value) << endl;
                    break;
                case WRONG_ID:
                    cerr << poset_doesnt_exist(func, id) << endl;
                    break;
                default:
                    cerr << element_already_exists_info(id, value) << endl;
                    break;
            }
        }
    }
}

void poset_remove_debug_info(unsigned long id, char const* value, ERROR err)
{
    if (debug) {
        string func = "poset_remove";
        if (err == NULL_VALUE) {
            cerr << call_info_two_params(func, id, "NULL") << endl;
            cerr << invalid_value(func, "value") << endl;
        }
        else {
            cerr << call_info_two_params(func, id, value) << endl;
            switch (err) {
                case WRONG_ID:
                    cerr << poset_doesnt_exist(func, id) << endl;
                    break;
                case NONEXISTING_VALUE:
                    cerr << element_doesnt_exist(id, value) << endl;
                    break;
                default:
                    cerr << element_removed(id, value) << endl;
                    break;
            }
        }
    }
}

// za c wstawiamy 'a', jeśli wywołujemy z funkcji poset_add;
//                'd', jeśli wywołujemy z funkcji poset_del;
//                't', jeśli wywołujemy z funkcji poset_test
void poset_add_del_test_debug_info(char c,
                                   unsigned long id,
                                   char const* value1,
                                   char const* value2,
                                   ERROR err)
{
    if (debug) {
        string func;
        switch (c) {
            case 'a':
                func = "poset_add";
                break;
            case 'd':
                func = "poset_del";
                break;
            default:
                func = "poset_test";
                break;
        }
        switch (err) {
            case NULL_VALUE1:
                cerr << call_info_three_params(func, id, "NULL", value2) << endl;
                cerr << invalid_value(func, "value1") << endl;
                break;
            case NULL_VALUE2:
                cerr << call_info_three_params(func, id, value1, "NULL") << endl;
                cerr << invalid_value(func, "value2") << endl;
                break;
            case NULL_VALUE:
                cerr << call_info_three_params(func, id, "NULL", "NULL") << endl;
                cerr << invalid_value(func, "value1") << endl;
                cerr << invalid_value(func, "value2") << endl;
                break;
            default:
                cerr << call_info_three_params(func, id, value1, value2) << endl;
                switch (err) {
                    case WRONG_ID:
                        cerr << poset_doesnt_exist(func, id) << endl;
                        break;
                    case NONEXISTING_VALUE:
                        cerr << nonexisting_value(func, id, value1, value2) << endl;
                        break;
                    case WRONG_RELATION:
                        switch (c) {
                            case 'a':
                                cerr << relation_info(func, id, value1, value2,
                                                      "cannot be added")
                                     << endl;
                                break;
                            case 'd':
                                cerr << relation_info(func, id, value1, value2,
                                                      "cannot be deleted")
                                     << endl;
                                break;
                            default:
                                cerr << relation_info(func, id, value1, value2,
                                                      "does not exist")
                                     << endl;
                                break;
                        }
                        break;
                    default:
                        switch (c) {
                            case 'a':
                                cerr << relation_info(func, id, value1, value2, "added")
                                     << endl;
                                break;
                            case 'd':
                                cerr << relation_info(func, id, value1, value2, "deleted")
                                     << endl;
                                break;
                            default:
                                cerr << relation_info(func, id, value1, value2, "exists")
                                     << endl;
                                break;
                        }
                        break;
                }
                break;
        }
    }
}

void poset_clear_debug_info(unsigned long id, ERROR err)
{
    if (debug) {
        string func = "poset_clear";
        cerr << call_info_one_param(func, id) << endl;
        if (err == WRONG_ID) {
            cerr << poset_doesnt_exist(func, id) << endl;
        }
        else if (err == NONE) {
            cerr << poset_info(func, id, "cleared") << endl;
        }
    }
}
    //============================================================================
}

unsigned long jnp1::poset_new(void)
{
    unsigned long id = new_poset_id;

    new_poset_id++;

    poset new_poset;

    posets().insert(make_pair(id, new_poset));
    identifiers().insert(
            make_pair(id, make_pair(0, unordered_map<string, int>())));

    poset_new_debug_info(id);

    return id;
}

void jnp1::poset_delete(unsigned long id)
{
    ERROR err = NONE;

    if (posets().count(id)) {
        posets().erase(id);
        identifiers().erase(id);
    }
    else
        err = WRONG_ID;

    poset_delete_debug_info(id, err);
}

size_t jnp1::poset_size(unsigned long id)
{
    size_t size = 0;
    ERROR err = NONE;

    if (posets().count(id))
        size = posets()[id].size();
    else
        err = WRONG_ID;

    poset_size_debug_info(id, size, err);

    return size;
}

bool jnp1::poset_insert(unsigned long id, const char* value)
{
    if (value == nullptr) {
        poset_insert_debug_info(id, value, NULL_VALUE);
        return false;
    }

    if (!posets().count(id)) {
        poset_insert_debug_info(id, value, WRONG_ID);
        return false;
    }

    string inscription(value);

    // w posecie już istnieje element o takiej nazwie
    if (identifiers()[id].second.count(value)) {
        poset_insert_debug_info(id, value, EXISTING_VALUE);
        return false;
    }
    else {
        identifiers()[id].first++;
        identifiers()[id].second.emplace(inscription, identifiers()[id].first);
        unordered_set<int> s1, s2;

        // dodaj relację do samego siebie
        int nodeID = identifiers()[id].second[inscription];
        s1.insert(nodeID);

        poset_value new_poset_value(s1, s2);
        posets()[id].insert(make_pair(identifiers()[id].first, new_poset_value));

        poset_insert_debug_info(id, value, NONE);
        return true;
    }
}

bool jnp1::poset_remove(unsigned long id, char const* value)
{
    if (posets().count(id) && value != nullptr) {
        poset& p = posets()[id];
        string deleted(value);

        // jeśli w oisecie nie ma elementu o takiej nazwie
        if (!identifiers()[id].second.count(deleted)) {
            poset_remove_debug_info(id, value, NONEXISTING_VALUE);
            return false;
        }

        int deleted_ID = identifiers()[id].second[deleted];

        // usuwamy relacje z następników
        for (auto& s : p[deleted_ID].first) {
            p[s].second.erase(deleted_ID);
        }

        // usuwamy relacje z poprzedników
        for (auto& s : p[deleted_ID].second) {
            p[s].first.erase(deleted_ID);
        }

        // usuwamy element c
        p.erase(deleted_ID);
        identifiers()[id].second.erase(deleted);

        poset_remove_debug_info(id, value, NONE);
        return true;
    }
    else {
        if (value == nullptr)
            poset_remove_debug_info(id, value, NULL_VALUE);
        else if (!posets().count(id))
            poset_remove_debug_info(id, value, WRONG_ID);

        return false;
    }
}

bool jnp1::poset_add(unsigned long id, char const* value1, char const* value2)
{
    if (value1 == nullptr || value2 == nullptr) {
        if (value1 == nullptr && value2 == nullptr)
            poset_add_del_test_debug_info('a', id, value1, value2, NULL_VALUE);
        else if (value1 == nullptr)
            poset_add_del_test_debug_info('a', id, value1, value2, NULL_VALUE1);
        else
            poset_add_del_test_debug_info('a', id, value1, value2, NULL_VALUE2);

        return false;
    }

    if (!posets().count(id)) {
        poset_add_del_test_debug_info('a', id, value1, value2, WRONG_ID);
        return false;
    }

    string s1(value1);
    string s2(value2);
    poset& tested = posets()[id];

    // pobierz identyfikatory elemtnów w posecie
    int i1, i2;

    if (!identifiers()[id].second.count(s1) || !identifiers()[id].second.count(s2)) {
        poset_add_del_test_debug_info('a', id, value1, value2, NONEXISTING_VALUE);
        return false;
    }

    i1 = identifiers()[id].second[s1];
    i2 = identifiers()[id].second[s2];

    if (tested[i1].first.count(i2) || tested[i2].first.count(i1)) {
        poset_add_del_test_debug_info('a', id, value1, value2, WRONG_RELATION);
        return false;
    }

    // stwórz wektor następników węzła reprezentowanego przez value2
    vector<int> consequents;
    queue<int> q;
    unordered_set<int> vis;
    q.push(i2);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        consequents.push_back(curr);

        for (auto& neighbor : tested[curr].first) {
            if (!vis.count(neighbor)) {
                q.push(neighbor);
                vis.insert(neighbor);
            }
        }
    }

    // dla każdego poprzednika value1 (wliczając go samego) dodaj krawędzie idące
    // do następników value2
    vis.clear();
    // kolejka jest pusta
    q.push(i1);

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        // dodaj nowe krawędzie do relacji
        for (auto& c : consequents) {
            tested[curr].first.emplace(c);
            tested[c].second.emplace(curr);
        }

        // dodaj do kolejki poprzedniki obecnego wierzchołka
        for (auto& neighbor : tested[curr].second) {
            if (!vis.count(neighbor)) {
                q.push(neighbor);
                vis.insert(neighbor);
            }
        }
    }

    poset_add_del_test_debug_info('a', id, value1, value2, NONE);
    return true;
}

bool jnp1::poset_del(unsigned long id, char const* value1, char const* value2)
{
    // jeśli któryś z wskaźników na wartości jest NULL-em
    if (value1 == nullptr || value2 == nullptr) {
        if (value1 == nullptr && value2 == nullptr)
            poset_add_del_test_debug_info('d', id, value1, value2, NULL_VALUE);
        else if (value1 == nullptr)
            poset_add_del_test_debug_info('d', id, value1, value2, NULL_VALUE1);
        else
            poset_add_del_test_debug_info('d', id, value1, value2, NULL_VALUE2);

        return false;
    }

    // jeśli nie istnieje poset o danym numerze
    if (!posets().count(id)) {
        poset_add_del_test_debug_info('d', id, value1, value2, WRONG_ID);
        return false;
    }

    string s1(value1);
    string s2(value2);
    poset& tested = posets()[id];

    // jeśli któraś z wartości nie istnieje
    if (!identifiers()[id].second.count(s1) || !identifiers()[id].second.count(s2)) {
        poset_add_del_test_debug_info('d', id, value1, value2, NONEXISTING_VALUE);
        return false;
    }

    int i1 = identifiers()[id].second[s1];
    int i2 = identifiers()[id].second[s2];

    // jeśli w danym posecie nie istnieje relacja, w której value1 poprzedza
    // value2
    if (!tested[i1].first.count(i2)) {
        poset_add_del_test_debug_info('d', id, value1, value2, WRONG_RELATION);
        return false;
    }

    // sprawdź, czy usunięcie relacji zakłóci przechodniość
    unordered_set<int> vis;
    queue<int> q;
    q.push(i1);
    bool isPathAvailable = false;

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        if (curr == i2) {
            isPathAvailable = true;
            break;
        }

        for (auto& neighbor : tested[curr].first) {
            // pomijamy krawędź, którą próbujemy usunąć
            if (curr != i1 || neighbor != i2) {
                if (!vis.count(neighbor)) {
                    // dodajemy krawędź do kolejki
                    q.push(neighbor);
                    vis.insert(neighbor);
                }
            }
        }
    }

    if (!isPathAvailable) {
        // możemy usunąć krawędź z relacji
        tested[i1].first.erase(i2);
        tested[i2].second.erase(i1);

        poset_add_del_test_debug_info('d', id, value1, value2, NONE);
        return true;
    }
    else {
        poset_add_del_test_debug_info('d', id, value1, value2, WRONG_RELATION);
        return false;
    }
}

bool jnp1::poset_test(unsigned long id,
                      char const* value1,
                      char const* value2)
{
    if (value1 != nullptr && value2 != nullptr && posets().count(id)) {
        poset& tested = posets()[id];
        string s1(value1);
        string s2(value2);
        int i1, i2;

        // jeśli któraś z wartości nie istnieje
        if (!identifiers()[id].second.count(s1) || !identifiers()[id].second.count(s2)) {
            poset_add_del_test_debug_info('t', id, value1, value2, NONEXISTING_VALUE);
            return false;
        }

        i1 = identifiers()[id].second[s1];
        i2 = identifiers()[id].second[s2];

        // testowanie relacji
        if (tested[i1].first.count(i2)) {
            poset_add_del_test_debug_info('t', id, value1, value2, NONE);
            return true;
        }
        else {
            poset_add_del_test_debug_info('t', id, value1, value2, WRONG_RELATION);
            return false;
        }
    }
    else {
        // jeśli któryś ze wskaźników na wartości jest NULL-em
        if (value1 == nullptr && value2 == nullptr)
            poset_add_del_test_debug_info('t', id, value1, value2, NULL_VALUE);
        else if (value1 == nullptr)
            poset_add_del_test_debug_info('t', id, value1, value2, NULL_VALUE1);
        else if (value2 == nullptr)
            poset_add_del_test_debug_info('t', id, value1, value2, NULL_VALUE2);
        // jeśli poset o danym numerze nie istnieje
        else
            poset_add_del_test_debug_info('t', id, value1, value2, WRONG_ID);

        return false;
    }
}

void jnp1::poset_clear(unsigned long id)
{
    if (posets().count(id)) {
        posets()[id].clear();
        identifiers()[id].first = 0;
        identifiers()[id].second.clear();

        poset_clear_debug_info(id, NONE);
    }
    else
        poset_clear_debug_info(id, WRONG_ID);
}
