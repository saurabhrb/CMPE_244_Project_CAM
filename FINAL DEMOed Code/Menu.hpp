/*
 * Menu.hpp
 *
 *  Created on: Apr 26, 2018
 *      Author: GyR0
 */

#ifndef MENU_HPP_
#define MENU_HPP_

#include "vector.hpp"
#include "printf_lib.h"
#include "str.hpp"
#include "stdint.h"

struct Element {
    bool type; //submenu:0/item:1
    int id;
    str name;
};

enum type {
    menuType, itemType
};

class Menu {
public:
    Menu();
    ~Menu();
    Menu(str nm, Menu* CurrentMenu, Menu* ParentMenu, type mType);

    //to add new sub-menu by its name to current menu object
    void addSubMenu(str nm);

    //to add new sub-item by its name to current menu object
    void addItem(str nm);

    //to find if element with given string exists in elemnts list
    bool isELEPresent(str nm, int &index);

    //get sub-menu object by its name, with found/no-found check
    bool getSubMenu(str nm, Menu* SubMenu);

    //get sub-item object by its name, with found/no-found check
    bool getItem(str nm, Menu* SubItem);

    //get sub-menu object reference by its name, MAKE SURE IT EXISTS IN YOUR Sub-Menus List!
    Menu* confirmedGetSubMenu(str nm);

    //get sub-item object reference by its name, MAKE SURE IT EXISTS IN YOUR Sub-Menus List!
    Menu* confirmedGetItem(str nm);

    //display all sub-menu/items of current menu object
    void dispallelements();

    //formatted display with menu heading as path
    virtual void Display();

    //to handle scroll down in current menu
    void scrollDown();

    //to handle scroll up in current menu
    void scrollUp();

    //to handle going inside the highlited menu object
    //it returns the reference to highlighted menu object
    Menu* loadNext();

    //to handle going outside the highlited menu object
    //it returns the reference to highlighted menu object's parentmenu
    Menu* loadPrev();

public:
    type myType; //to know if current object is itemType or menuType
    str myName; //name of this object

    Menu *ParentMenu; //parent menu of current menu object
    Menu *CurrentMenu; //reference to global CurrentMenuG

    VECTOR<Menu*> SubMenus; //list of all sub-menu objects inside current menu object

    int highlighted; //to track currently highlighted sub-element

    VECTOR<Element> all_elements; //to store all elements by Element .name .id and .type
    VECTOR<str> all_elements_name; //to store all elements by name to use by Find function
};

#endif /* MENU_HPP_ */
