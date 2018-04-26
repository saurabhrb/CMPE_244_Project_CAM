/******************************************************************************

                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

struct Element
{
bool type; //submenu(0)/item(1)
int id;
string name;
};

enum type{menuType, itemType};

class Menu{
public:
Menu(){
	highlighted = 0;
	myType = menuType;
	this->ParentMenu = nullptr;
	this->CurrentMenu = nullptr;
	myName = "NULL";
}

Menu(string nm, Menu* CurrentMenu, Menu* ParentMenu, type mType){
myName = nm;
this->ParentMenu = ParentMenu;
this->CurrentMenu = CurrentMenu;
highlighted = 0;
myType = mType;
}
type myType;
string myName;

Menu *ParentMenu;
Menu *CurrentMenu;

vector<Menu*> SubMenus;

int highlighted;

vector<Element> all_elements;
vector<string> all_elements_name;

void addSubMenu(string nm)
{
    int ind = -1;
	if(isELEPresent(nm,ind))
		nm = nm + "_";
	Menu *submenu = new Menu(nm, this->CurrentMenu, this, menuType);
	SubMenus.push_back(submenu);
	Element ele1;
	ele1.type = false; ele1.id = (int)all_elements.size(); ele1.name = nm;
	all_elements.push_back(ele1);
	all_elements_name.push_back(nm);
}

void addItem(string nm)
{
    int ind = -1;
	if(isELEPresent(nm,ind))
		nm = nm + "_";
	Menu *subitem = new Menu(nm, this->CurrentMenu, this,itemType);
	SubMenus.push_back(subitem);
	Element ele1;
	ele1.type = true; ele1.id = (int)all_elements.size(); ele1.name = nm;
	all_elements.push_back(ele1);
	all_elements_name.push_back(nm);
}

bool isELEPresent(string nm,int &index)
{
	bool present = true;
	std::vector<string>::iterator it;
	it = find(all_elements_name.begin(), all_elements_name.end(), nm);
	if (it != all_elements_name.end())
	{
		 index = std::distance(all_elements_name.begin(), it);
		return true;
	}
	else
	{
		return false;
	}
}

bool getSubMenu(string nm,Menu* SubMenu)
{
	int ind = -1;
	if(isELEPresent(nm,ind))
	{
		SubMenu = SubMenus[ind];
		return true;
	}
	
	return false;
}


bool getItem(string nm,Menu* SubItem)
{
	int ind = -1;
	if(isELEPresent(nm,ind))
	{
		SubItem = SubMenus[ind];
		return true;
	}
	
	return false;
}

Menu* confirmedGetSubMenu(string nm)
{
	int ind = -1;
	isELEPresent(nm,ind);
	return SubMenus[ind];	
}

Menu* confirmedGetItem(string nm)
{
	int ind=-1;
	isELEPresent(nm,ind);
	return SubMenus[ind];	
}

void dispallelements()
{
    cout<<"\n\n== "<<myName<<" Elements==\n\n";
    cout<<"highlighted = "<<highlighted<<"\n";
    
    for(int i=0;i<all_elements.size();i++)
	{
		cout<<((all_elements[i].type==itemType)?"":"+")<<all_elements[i].id<<":"<<all_elements[i].name<<"\n";
	}
    cout<<"\n\n";
    
}
void Display()
{
	Menu *p = ParentMenu;
	string path = this->myName;
	while(p)
	{
		path = p->myName + "->" + path;
		p=p->ParentMenu;
	}
	cout<<"\n\n======\n"<<path<<"\n==========\n";
	
	for(int i=0;i<all_elements.size();i++)
	{
	    if(highlighted==i)
	        cout<<">";
	        
		cout<<((all_elements[i].type==itemType)?"":"+")<<all_elements[i].id<<":"<<all_elements[i].name<<"\n";
	}
	cout<<"\n\n\n";
}

void scrollDown()
{
	highlighted++;
	highlighted = highlighted%all_elements.size();
}

void scrollUp()
{
	highlighted--;
	if(highlighted<0)
	    highlighted=0;
}

Menu* loadNext()
{
	if(SubMenus[highlighted]->myType == itemType)
		cout<<"\n\nEXECUTED ITEM : "<<SubMenus[highlighted]->myName<<"()\n\n";
	else
	{
		if(this->SubMenus[highlighted]->all_elements.size()==0)
			{
			    cout<<"\n\nSUB-MENU EMPTY\n\n";
			}
		else{
			return this->SubMenus[highlighted];
		}
	}
	return this;
}
	
Menu* loadPrev()
{
    if(ParentMenu==nullptr)
        cout<<"\n\nALREADY IN MAIN MENU\n\n";
    else
        return ParentMenu;
	return this;
}	
};


int main()
{
	Menu *CurrentMenuG;
	Menu *MAIN = new Menu((string)"MAIN",CurrentMenuG, nullptr, menuType);
	
	//Make the MAIN MENU
	MAIN->addSubMenu("Sub1");
	MAIN->addSubMenu("Sub2");
	MAIN->addItem("It1");
	MAIN->addItem("It2");

    //Sub1
    Menu *Sub1 = MAIN->confirmedGetSubMenu("Sub1");
    Sub1->addSubMenu("Sub1_Sub1");
	Sub1->addSubMenu("Sub1_Sub2");
	Sub1->addItem("Sub1_It1");
	Sub1->addItem("Sub1_It2");
        
    //Sub1_Sub1
    Menu *Sub1_Sub1 = Sub1->confirmedGetSubMenu("Sub1_Sub1");
    Sub1_Sub1->addSubMenu("Sub1_Sub1_Sub1");
	Sub1_Sub1->addItem("Sub1_Sub1_It1");
    
    //Sub2
    Menu *Sub2 = MAIN->confirmedGetSubMenu("Sub2");
    Sub2->addSubMenu("Sub2_Sub1");
	Sub2->addSubMenu("Sub2_Sub2");
	Sub2->addItem("Sub2_It1");
	Sub2->addItem("Sub2_It2");
    
    CurrentMenuG = MAIN;
	char c;
	bool exit = false;
	do
	{
	    CurrentMenuG -> Display();
		cin>>c;
		switch(c)
		{
			case 'e':
				cout<<"\n\nEXITED...\n";
				exit = true;
				break;
			case 'w':
			    cout<<"scrollUp";
				CurrentMenuG->scrollUp();
				break;
			case 's':
				cout<<"scrollDown";
				CurrentMenuG->scrollDown();
				break;
			case 'a':
			    cout<<"Prev";
				CurrentMenuG = CurrentMenuG->loadPrev();
				break;
			case 'd':
			    cout<<"Next";
				CurrentMenuG = CurrentMenuG->loadNext();
				break;
			default:
				cout<<"\n\nWRONG ENTRY\nRE-ENTER...\n";
				break;
		}
		
	}while(exit == false);
	return 0;
}
