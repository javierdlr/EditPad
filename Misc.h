#ifndef MISC_H
#define MISC_H 1


//#define GetHead(list)  (struct Node *)list->lh_Head
//#define GetSucc(node)  node->ln_Succ


void Set_WindowTitle(struct editpad_data *);
void Read_ToolTypes(struct editpad_data *);
void Split_FullName(struct editpad_data *);
void Save_File(struct editpad_data *);
BOOL Load_File(struct editpad_data *);
LONG FileRequest_ASL(struct editpad_data *, CONST_STRPTR title, BOOL save);
void Print_File(struct editpad_data *);


#endif
