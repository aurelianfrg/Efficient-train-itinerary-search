#include <list>

template<typename T>
void sorted_list_insertion(list<T> liste, const T & element) {
    
    // Find the correct position to insert the value
    auto it = liste.begin();
    while (it != liste.end() && *it < element) {
        ++it;
    }
    // Insert the value at the found position
    liste.insert(it, element);

};