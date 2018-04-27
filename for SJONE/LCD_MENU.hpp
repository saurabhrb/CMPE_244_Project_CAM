/*
 * LCD_MENU.hpp
 *
 *  Created on: Apr 26, 2018
 *      Author: GyR0
 */

#ifndef LCD_MENU_HPP_
#define LCD_MENU_HPP_

#include "LCD.hpp"
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

class LCD_MENU {
private:
    type myType; //to know if current object is itemType or menuType
    str myName; //name of this object

    LCD_MENU *ParentMenu; //parent menu of current menu object
    LCD_MENU *CurrentMenu; //reference to global CurrentMenuG

    VECTOR<LCD_MENU*> SubMenus; //list of all sub-menu objects inside current menu object

private:
    int highlighted; //to track currently highlighted sub-element

    VECTOR<Element> all_elements; //to store all elements by Element .name .id and .type
    VECTOR<str> all_elements_name; //to store all elements by name to use by Find function
public:
    LCD *lcd;
    LCD_MENU(LabGPIO_X CS, LabGPIO_X RST, unsigned int max_clock_mhz, str nm, LCD_MENU* CurrentMenu_, LCD_MENU* ParentMenu_, type mType);

    LCD_MENU(LCD *lcd, str nm, LCD_MENU* CurrentMenu_, LCD_MENU* ParentMenu_, type mType);

    LCD_MENU();

    ~LCD_MENU();

    //to reset highlight to 0th position
    void highLight0();

    //to add new sub-menu by its name to current menu object
    void addSubMenu(str nm)
    {
        int ind = -1;
        if (isELEPresent(nm, ind)) nm = nm + "_";
        LCD_MENU *submenu = new LCD_MENU(lcd, nm, this->CurrentMenu, this, menuType);
        SubMenus.push_back(submenu);
        Element ele1;
        ele1.type = false;
        ele1.id = (int) all_elements.size();
        ele1.name = nm;
        all_elements.push_back(ele1);
        all_elements_name.push_back(nm);
    }
    //to add new sub-item by its name to current menu object

    void addItem(str nm)
    {
        int ind = -1;
        if (isELEPresent(nm, ind)) nm = nm + "_";
        LCD_MENU *subitem = new LCD_MENU(lcd, nm, this->CurrentMenu, this, itemType);
        SubMenus.push_back(subitem);
        Element ele1;
        ele1.type = true;
        ele1.id = (int) all_elements.size();
        ele1.name = nm;
        all_elements.push_back(ele1);
        all_elements_name.push_back(nm);
    }

    //to find if element with given string exists in elemnts list
    bool isELEPresent(str nm, int &index);

    //get sub-menu object by its name, with found/no-found check
    bool getSubMenu(str nm, LCD_MENU* SubMenu);

    //get sub-item object by its name, with found/no-found check
    bool getItem(str nm, LCD_MENU* SubItem);

    //get sub-menu object reference by its name, MAKE SURE IT EXISTS IN YOUR Sub-Menus List!
    LCD_MENU* confirmedGetSubMenu(str nm);

    //get sub-item object reference by its name, MAKE SURE IT EXISTS IN YOUR Sub-Menus List!
    LCD_MENU* confirmedGetItem(str nm);

    //display all sub-menu/items of current menu object
    void dispallelements();

    //formatted display with menu heading as path
    void Display();

    //to handle scroll down in current menu
    void scrollDown();

    //to handle scroll up in current menu
    void scrollUp();

    //to handle going inside the highlited menu object
    //it returns the reference to highlighted menu object
    LCD_MENU* loadNext();

    //to handle going outside the highlited menu object
    //it returns the reference to highlighted menu object's parentmenu
    LCD_MENU* loadPrev();

    //loads the image
    void clear_disp(uint16_t photo[]);
};

#endif /* LCD_MENU_HPP_ */
