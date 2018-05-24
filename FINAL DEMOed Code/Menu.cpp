/*
 * Menu.cpp
 *
 *  Created on: Apr 26, 2018
 *      Author: GyR0
 */

#include "Menu.hpp"

Menu::Menu()
{
    highlighted = 0;
    myType = menuType;
    this->ParentMenu = nullptr;
    this->CurrentMenu = nullptr;
    myName = "NULL";
}

Menu::~Menu()
{

}

Menu::Menu(str nm, Menu* CurrentMenu_, Menu* ParentMenu_, type mType)
{
    myName = nm;
    this->ParentMenu = ParentMenu_;
    this->CurrentMenu = CurrentMenu_;
    highlighted = 0;
    myType = mType;
}

void Menu::addSubMenu(str nm)
{
    int ind = -1;
    if (isELEPresent(nm, ind)) nm = nm + "_";
    Menu *submenu = new Menu(nm, this->CurrentMenu, this, menuType);
    SubMenus.push_back(submenu);
    Element ele1;
    ele1.type = false;
    ele1.id = (int) all_elements.size();
    ele1.name = nm;
    all_elements.push_back(ele1);
    all_elements_name.push_back(nm);
}

void Menu::addItem(str nm)
{
    int ind = -1;
    if (isELEPresent(nm, ind)) nm = nm + "_";
    Menu *subitem = new Menu(nm, this->CurrentMenu, this, itemType);
    SubMenus.push_back(subitem);
    Element ele1;
    ele1.type = true;
    ele1.id = (int) all_elements.size();
    ele1.name = nm;
    all_elements.push_back(ele1);
    all_elements_name.push_back(nm);
}

bool Menu::isELEPresent(str nm, int &index)
{
    for (uint8_t i = 0; i < ((uint8_t) all_elements_name.size()); i++) {
        if (all_elements_name[i] == nm) {
            index = i;
            return true;
        }
    }
    return false;

}

bool Menu::getSubMenu(str nm, Menu* SubMenu)
{
    int ind = -1;
    if (isELEPresent(nm, ind)) {
        SubMenu = SubMenus[ind];
        return true;
    }

    return false;
}

bool Menu::getItem(str nm, Menu* SubItem)
{
    int ind = -1;
    if (isELEPresent(nm, ind)) {
        SubItem = SubMenus[ind];
        return true;
    }

    return false;
}

Menu* Menu::confirmedGetSubMenu(str nm)
{
    int ind = -1;
    isELEPresent(nm, ind);
    return SubMenus[ind];
}

Menu* Menu::confirmedGetItem(str nm)
{
    int ind = -1;
    isELEPresent(nm, ind);
    return SubMenus[ind];
}

void Menu::dispallelements()
{
    u0_dbg_printf("\n\n== %s Elements==\n\n", myName.c_str());
    u0_dbg_printf("highlighted = %d\n", highlighted);

    for (uint8_t i = 0; i < ((uint8_t) all_elements.size()); i++) {
        u0_dbg_printf("%c%d: %s\n", ((all_elements[i].type == itemType) ? ' ' : '+'), all_elements[i].id, all_elements[i].name.c_str());
    }
    u0_dbg_printf("\n\n");

}
void Menu::Display()
{
    Menu *p = ParentMenu;
    str path = this->myName;
    while (p) {
        path.insertAtBeg("->");
        path.insertAtBeg(p->myName);
        p = p->ParentMenu;
    }
    u0_dbg_printf("\n\n======\n%s\n==========\n", path.c_str());

    for (uint8_t i = 0; i < ((uint8_t) all_elements.size()); i++) {
        if (highlighted == i) u0_dbg_printf(">");
        else
            u0_dbg_printf(" ");

        if (all_elements[i].type == itemType) u0_dbg_printf(" ");
        else
            u0_dbg_printf("+");

        u0_dbg_printf("%d:%s\n", all_elements[i].id, all_elements[i].name.c_str());
    }
    u0_dbg_printf("\n\n\n");
}

void Menu::scrollDown()
{
    highlighted++;
    highlighted = highlighted % all_elements.size();
}

void Menu::scrollUp()
{
    highlighted--;
    if (highlighted < 0) highlighted = all_elements.size() - 1;
}

Menu* Menu::loadNext()
{
    if (SubMenus[highlighted]->myType == itemType) u0_dbg_printf("\n\nEXECUTED ITEM : %s()\n\n", SubMenus[highlighted]->myName.c_str());
    else {
        if (this->SubMenus[highlighted]->all_elements.size() == 0) {
            u0_dbg_printf("\n\nSUB-MENU EMPTY\n\n");
        }
        else {
            return this->SubMenus[highlighted];
        }
    }
    return this;
}

Menu* Menu::loadPrev()
{
    if (ParentMenu == nullptr) u0_dbg_printf("\n\nALREADY IN MAIN MENU\n\n");
    else
        return ParentMenu;
    return this;
}
