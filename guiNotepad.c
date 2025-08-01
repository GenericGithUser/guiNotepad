#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iup.h>
#include <iup_config.h>
#include <io.h>

Ihandle *textBox = NULL;
Ihandle *diagBox = NULL;
Ihandle* g_config = NULL;

char filePath[2048] = "";

const char *getFileTitle(const char *fileName){

    int len = (int)strlen(fileName);
    int offset = len - 1;

    while (offset != 0)
    {
        if (fileName[offset] == '\\' || fileName[offset] == '/')
        {
            offset++;
            break;
        }
        offset--;
    }
    
    return fileName + offset;
}

int compString(const char *str1, const char *str2, int isCaseSens){
    if (!str1 || !str2)
    {
        return 0;
    }
    int length = (int)strlen(str2);

    if (isCaseSens)
    {
        return strncmp(str1, str2, length) == 0;
    }
    
    #ifdef _WIN32
        return _strnicmp(str1, str2, length) == 0;
    #else 
        return strncasecmp(str1, str2, length) == 0;
    #endif

}

int findStr(const char *str, const char *strToFind, int isCaseSens){

    int i, strLen, strtoFindLen, count;

    if (!str || str[0] == 0 || !strToFind || strToFind[0] == 0)
    {
        return -1;
    }
    
    strLen = (int)strlen(str);
    strtoFindLen = (int)strlen(strToFind);
    count = strLen - strtoFindLen;

    if(count < 0){
        return -1;
    }
    count++;

    for (i = 0; i < count; i++)
    {
        if (compString(str, strToFind, isCaseSens))
        {
            return i;
        }

        str++;
        
    }
    return -1;
}

char* readFile(const char *fileName){
    int size;
    char* str;

    FILE *fp = fopen(fileName, "rb");

    if(!fp){
        IupMessagef("Error", "Cannot Open File: %s", fileName);
    }

    // Calculate filesize

    fseek(fp, 0, SEEK_END); // seeks end file
    size = ftell(fp); // checks size based on pointer location
    fseek(fp, 0, SEEK_SET); // returns file pointer to beginning

    // memalloc
    str = malloc(size + 1); // sets memory based on size of file + \0
    fread(str, size, 1, fp); // reads the entire file once
    str[size] = 0;

    if(ferror(fp)){
        IupMessagef("Error", "Failed to Read File: %s", fileName);
        return NULL;
    }

    fclose(fp);
    return str;
}

int writeFile(const char *fileName, const char *str, int count){

    FILE *fp = fopen(fileName, "w");
    if(!fp){
        IupMessagef("Error", "Cannot Open File: %s", fileName);
    }

    fwrite(str, 1, count, fp); // writes file

    if(ferror(fp)){
        IupMessagef("Error", "Failed Writing into File: %s", fileName);
    }

    fclose(fp);
    return 1;
    
}

void newFile(Ihandle *ih){

    IupSetAttribute(diagBox, "TITLE", "Untitled - Notepad");
    IupSetAttribute(textBox, "FILENAME", NULL);
    IupSetAttribute(textBox, "DIRTY", "NO");
    IupSetAttribute(textBox, "VALUE", "");
}

void openSubFunction(Ihandle *ih, const char *fileName){

    char *str = readFile(fileName);
        if (str){

            IupSetfAttribute(diagBox, "TITLE", "%s - Notepad", getFileTitle(fileName));
            IupSetfAttribute(textBox, "FILENAME", fileName);
            IupSetAttribute(textBox, "DIRTY", "NO");
            IupSetStrAttribute(textBox, "VALUE", str);
            
            strcpy(filePath, fileName);
            IupConfigRecentUpdate(g_config, filePath);
            free(str);
            
        }
        

}
void saveAsSubFunc(const char *fileName){
    char *str = IupGetAttribute(textBox, "VALUE");
    int count = IupGetInt(textBox, "COUNT");
    if (writeFile(fileName, str, count))
        {
            IupSetfAttribute(diagBox, "TITLE", "%s - Notepad", getFileTitle(fileName));
            IupSetStrAttribute(textBox, "FILENAME", fileName);
            IupSetAttribute(textBox, "DIRTY", "NO");
            strcpy(filePath, fileName);
            if(g_config){
                IupConfigRecentUpdate(g_config, filePath);
            IupConfigSetVariableStr(g_config, "Force", "Written", "yes");
            }
        }
}

int save(void);

int saveCheck(Ihandle *ih){
    if (IupGetInt(textBox, "DIRTY"))
    {
        switch (IupAlarm("Warning", "File Not Saved! Save it Now?", "Yes", "No", "Cancel"))
        {
        case 1:
            save();
            break;

        case 2:
            break;

        case 3:
            return 0;
        }
    }
    return 1;
    
}

//   -------    CALLBACKS  -------


int dropFile(Ihandle *ih, const char *fileName){
    if (saveCheck(ih))
    {
        openSubFunction(ih, fileName);
    }
    return IUP_DEFAULT;
    
}

int textBoxChanged(){
    
    IupSetAttribute(textBox, "DIRTY", "YES");
    return IUP_DEFAULT;

}

int fileMenuCallback(Ihandle *ih){

    Ihandle *itemRevert = IupGetDialogChild(ih, "ITEM_REVERT");
    Ihandle *itemSave = IupGetDialogChild(ih, "ITEM_SAVE");

    char *fileName = IupGetAttribute(textBox, "FILENAME");
    int dirty = IupGetInt(textBox, "DIRTY");

    if ("DIRTY")
    {
        IupSetAttribute(itemSave, "ACTIVE", "YES");
    }
    else{
        IupSetAttribute(itemSave, "ACTIVE", "NO");
    }

    if (dirty && fileName)
    {
        IupSetAttribute(itemRevert, "ACTIVE", "YES");

    }
    else{
        IupSetAttribute(itemRevert, "ACTIVE", "NO");
    }


    return IUP_DEFAULT;
    
}


int configCallback(Ihandle *ih){

    if (saveCheck(ih))
    {
        char *fileName = IupGetAttribute(ih, "RECENTFILENAME");
        openSubFunction(ih, fileName);
    }
    
    return IUP_DEFAULT;
    
}



int openFile(Ihandle *itemOpen){
    
    if (!saveCheck(itemOpen))
    {
        return IUP_DEFAULT;
    }
    Ihandle *fileDiag = IupFileDlg(); // opens built in file selector

    IupSetStrAttribute(fileDiag, "DIALOGTYPE", "OPEN");
    IupSetAttribute(fileDiag, "EXTILTER", "Text Files|*.txt|All Files|*.*|");
    IupSetAttributeHandle(fileDiag, "PARENTDIALOG", IupGetDialog(itemOpen));

    IupPopup(fileDiag, IUP_CENTER, IUP_CENTER);

    // check if user cancels
    if (IupGetInt(fileDiag, "STATUS") != -1){
        char *fileName = IupGetAttribute(fileDiag, "VALUE");

        openSubFunction(itemOpen, fileName);
        
    }

    IupDestroy(fileDiag);
    return IUP_DEFAULT;
}


int saveAs(Ihandle *parentDiag){
    Ihandle *fileDiag = IupFileDlg(); // opens built in file selector
    IupSetStrAttribute(fileDiag, "DIALOGTYPE", "SAVE");
    IupSetAttribute(fileDiag, "EXTILTER", "Text Files|*.txt|All Files|*.*|");
    if (parentDiag)
    {
        IupSetAttributeHandle(fileDiag, "PARENTDIALOG", IupGetDialog(parentDiag));
    }
    
    IupSetStrAttribute(fileDiag, "FILE", IupGetAttribute(textBox, "FILENAME"));

    IupPopup(fileDiag, IUP_CENTER, IUP_CENTER);

    // check if users cancels
    if (IupGetInt(fileDiag, "STATUS") != -1){
        
        char *fileName = IupGetAttribute(fileDiag, "VALUE");

        if(!strchr(fileName, '.')){
            // update file name with proper extension
            static char updateFileName[1024]; // static to maintain filename for duration of program
            snprintf(updateFileName, sizeof(updateFileName), "%s.txt", fileName);
            fileName = updateFileName;
        }
      
        saveAsSubFunc(fileName);
        
       
    }

    IupDestroy(fileDiag);
    return IUP_DEFAULT;
}

int save(void){
    if (strlen(filePath) == 0)
    {
        return saveAs(diagBox);
    }
    
    char *str = IupGetAttribute(textBox, "VALUE");
    int count = strlen(str);

    if (writeFile(filePath, str, count))
    {
        IupSetAttribute(textBox, "DIRTY", "NO");
    }
    

    
    if (g_config)
    {
       IupConfigRecentUpdate(g_config, filePath);
       IupConfigSetVariableStr(g_config, "Force", "Written", "yes");
    }
    

    return IUP_DEFAULT;
}

int newFileCallback(Ihandle *itemNew){
    if (saveCheck(itemNew))
    {
        newFile(itemNew);
    }
    return IUP_DEFAULT;
}

int revertAct(Ihandle *itemRevert){

    char *fileName = IupGetAttribute(textBox, "FILENAME");
    openSubFunction(itemRevert, fileName);
    return IUP_DEFAULT;
}

int editMenuCallback(Ihandle *ih){
    Ihandle *clipboard = IupClipboard();
    Ihandle *itemPaste = IupGetDialogChild(ih, "ITEM_PASTE");
    Ihandle *itemCut = IupGetDialogChild(ih, "ITEM_CUT");
    Ihandle *itemCopy = IupGetDialogChild(ih, "ITEM_COPY");
    Ihandle *itemDelete = IupGetDialogChild(ih, "ITEM_DEL");

    int start, end;

    if (!IupGetInt(clipboard, "TEXTAVAILABLE"))
    {
        IupSetAttribute(itemPaste, "ACTIVE", "NO");
    }
    else{
        IupSetAttribute(itemPaste, "ACTIVE", "YES");
    }

    IupGetIntInt(textBox, "SELECTIONPOS", &start, &end);

    if (start == end)
    {
        IupSetAttribute(itemCut, "ACTIVE", "NO");
        IupSetAttribute(itemCopy, "ACTIVE", "NO");
        IupSetAttribute(itemDelete, "ACTIVE", "NO");
    }
    else{
        IupSetAttribute(itemCut, "ACTIVE", "YES");
        IupSetAttribute(itemCopy, "ACTIVE", "YES");
        IupSetAttribute(itemDelete, "ACTIVE", "YES");
    }
    
    IupDestroy(clipboard);
    return IUP_DEFAULT;
    
}

int editCopyCallback(Ihandle *itemCopy){

    Ihandle *clipboard = IupClipboard();
    IupSetAttribute(clipboard, "TEXT" , IupGetAttribute(textBox, "SELECTEDTEXT"));
    IupDestroy(clipboard);
    return IUP_DEFAULT;
}

int editCutCallback(Ihandle *itemCut){

    Ihandle *clipboard =IupClipboard();
    IupSetAttribute(clipboard, "TEXT" , IupGetAttribute(textBox, "SELECTEDTEXT"));
    IupSetAttribute(textBox, "SELECTEDTEXT", "");
    IupDestroy(clipboard);
    
    return IUP_DEFAULT;
    
}

int editDeleteCallback(Ihandle *itemDelete){

    IupSetAttribute(textBox, "SELECTEDTEXT", "");

    return IUP_DEFAULT;
    
}

int editPasteCallback(Ihandle *itemPaste){

    Ihandle *clipboard =IupClipboard();
    IupSetAttribute(textBox, "INSERT", IupGetAttribute(clipboard, "TEXT"));
    IupDestroy(clipboard);

    return IUP_DEFAULT;

}
int editSelectAllCallback(Ihandle *itemSelAll){

    IupSetFocus(textBox);
    IupSetAttribute(textBox, "SELECTION", "ALL");
    return IUP_DEFAULT;
}

int modFont(void){
    Ihandle *fontDiag = IupFontDlg();
    char *font = IupGetAttribute(textBox, "FONT");
    IupSetStrAttribute(fontDiag, "VALUE", font);
    IupPopup(fontDiag, IUP_CENTER, IUP_CENTER);

    if (IupGetInt(fontDiag, "STATUS") == 1)
    {
        char *font = IupGetAttribute(fontDiag, "VALUE");
        IupSetAttribute(textBox, "FONT", font);

        IupConfigSetVariableStr(g_config, "MainWindow", "Font", font);
    }

    IupDestroy(fontDiag);
    return IUP_DEFAULT;
}

int showAbout(void){
    IupMessage("About", "   Tutorial Notepad\n  Using IUP (Portable User Interface)\n   By: Gustavo Lyrio & Antonio Scuri\n\nMade by: GenericGithUser");
    return IUP_DEFAULT;
}


int gotoOkCallback(Ihandle *btnOk){
    int lineCnt = IupGetInt(btnOk, "TEXT_LINECOUNT");
    Ihandle *txt = IupGetDialogChild(btnOk, "LINE_TEXT");
    int line = IupGetInt(txt, "VALUE");
    if (line < 1 || line >= lineCnt){
        IupMessage("Error", "Invalid Line Number");
        return IUP_DEFAULT;
    }

    IupSetAttribute(IupGetDialog(btnOk), "STATUS", "1");
    return IUP_CLOSE;
}

int gotoCancelCallback(Ihandle *btnOk){
    IupSetAttribute(IupGetDialog(btnOk), "STATUS", "0");
    return IUP_CLOSE;
}

int gotoMainCallback(Ihandle *itemGoto){

    Ihandle *dlg, *box, *btnOk, *btnCnl, *txt, *label;

    int lineCount = IupGetInt(textBox, "LINECOUNT");

    label = IupLabel(NULL);
    IupSetfAttribute(label, "TITLE", "Line Number [1-%d]: ", lineCount);

    txt = IupText(NULL);
    IupSetAttribute(txt, "MASK", IUP_MASK_UINT);
    IupSetAttribute(txt, "NAME", "LINE_TEXT");
    IupSetAttribute(txt, "VISIBLECOLUMNS", "20");

    btnOk = IupButton("OK", NULL);
    IupSetInt(btnOk, "TEXT_LINECOUNT", lineCount);
    IupSetAttribute(btnOk, "PADDING", "10x2");
    IupSetCallback(btnOk, "ACTION", (Icallback)gotoOkCallback);

    btnCnl = IupButton("Cancel", NULL);
    IupSetAttribute(btnCnl, "PADDING", "10x2");
    IupSetCallback(btnCnl, "ACTION", (Icallback)gotoCancelCallback);

    box = IupVbox(
        label,
        txt,
        IupSetAttributes(
            IupHbox(
                IupFill(),
                btnOk,
                btnCnl,
                NULL
            )
            , "NORMALSIZE=HORIZONTAL"
        ),
    NULL);

    IupSetAttribute(box, "MARGIN", "10x10");
    IupSetAttribute(box, "GAP", "5");

    dlg = IupDialog(box);
    IupSetAttribute(dlg, "TITLE", "Go to Line..");
    IupSetAttribute(dlg, "DIALOGFRAME", "Yes");
    IupSetAttributeHandle(dlg, "DEFAULTENTER", btnOk);
    IupSetAttributeHandle(dlg, "DEFAULTESC", btnCnl);
    IupSetAttributeHandle(dlg, "PARENTDIALOG", IupGetDialog(itemGoto));

    IupPopup(dlg, IUP_CENTERPARENT, IUP_CENTERPARENT);

    if(IupGetInt(dlg, "STATUS") == 1){
        int line = IupGetInt(txt, "VALUE");
        int pos;
        IupTextConvertLinColToPos(textBox, line, 0, &pos);
        IupSetInt(textBox, "CARETPOS", pos);
        IupSetInt(textBox, "SCROLLPOS", pos);
    }

    IupDestroy(dlg);
    return IUP_DEFAULT;
    
}

int findNextCallback(Ihandle *btnNext){

    char *str = IupGetAttribute(textBox, "VALUE");
    int findPosition = IupGetInt(textBox,"FIND_POS");

    Ihandle *txt = IupGetDialogChild(btnNext, "FIND_TEXT");
    char *strToFind = IupGetAttribute(txt, "VALUE");

    Ihandle *findCase = IupGetDialogChild(btnNext, "FIND_CASE");
    int isCaseSens = IupGetInt(findCase, "VALUE");

    int pos = findStr(str + findPosition, strToFind, isCaseSens);

    if(pos >= 0){
        pos += findPosition;
    }
    else if(findPosition > 0){
        pos = findStr(str + findPosition, strToFind, isCaseSens);
    }

    if (pos >= 0)
    {
        int ln, col, endPos = pos + (int)strlen(strToFind);

        IupSetInt(textBox, "FIND_POS", endPos);

        IupSetFocus(textBox);
        IupSetfAttribute(textBox, "SELECTIONPOS", "%d:%d", pos, endPos);

        IupTextConvertPosToLinCol(textBox, pos, &ln, &col);
        IupTextConvertLinColToPos(textBox, ln, 0, &pos);
        IupSetInt(textBox, "SCROLLTOPOS", pos);
    }
    else{
        IupMessage("Not Found", "Text Not Found");
    }
    
    return IUP_DEFAULT;
}

int findCloseCallback(Ihandle *btnClose){

    IupHide(IupGetDialog(btnClose));
    return IUP_DEFAULT;
}

int findMainCallback(Ihandle *itemFind){

    Ihandle *diag = (Ihandle*)IupGetAttribute(itemFind, "FIND_DIALOG");
    if (!diag)
    {
        Ihandle *box, *btnNext, *btnClose, *searchBox, *isCaseSens;

        searchBox = IupText(NULL);
        IupSetAttribute(searchBox, "NAME", "FIND_TEXT");
        IupSetAttribute(searchBox, "VISIBLECOLUMNS", "20");

        isCaseSens = IupToggle("Case Sensitive", NULL);
        IupSetAttribute(isCaseSens, "NAME", "FIND_CASE");

        btnNext = IupButton("Find Next", NULL);
        IupSetAttribute(btnNext, "PADDING", "10x2");
        IupSetCallback(btnNext, "ACTION", (Icallback)findNextCallback);

        btnClose = IupButton("Cancel", NULL);
        IupSetAttribute(btnClose, "PADDING", "10x2");
        IupSetCallback(btnClose, "ACTION", (Icallback)findCloseCallback);

        box = IupVbox(
            IupLabel("Find What:"),
            searchBox,
            isCaseSens,
            IupSetAttributes(
                IupHbox(
                    IupFill(),
                    btnNext,
                    btnClose,
                    NULL
                ), "NORMALSIZE=HORIZONTAL"
            ),
        NULL);

        IupSetAttribute(box, "MARGIN", "10x10");
        IupSetAttribute(box, "GAP", "5");

        diag = IupDialog(box);
        IupSetAttribute(diag, "TITLE", "Find");
        IupSetAttribute(diag, "DIALOGFRAME", "Yes");

        IupSetAttributeHandle(diag, "DEFAULTENTER", btnNext);
        IupSetAttributeHandle(diag, "DEAFULTESC", btnClose);
        IupSetAttributeHandle(diag, "PARENTDIALOG", IupGetDialog(itemFind));

        // Saving textbox, will remove if not compat if with global var
        IupSetAttribute(diag, "MULTITEXT", (char*)textBox);

        IupSetAttribute(itemFind, "FIND_DIALOG", (char*)diag);
    }

    IupShowXY(diag, IUP_CURRENT, IUP_CURRENT);

    return IUP_DEFAULT;
    
}


int textBoxPointer(Ihandle *ih, int line, int col){
    Ihandle *statBar = IupGetDialogChild(ih, "STATUSBAR");
    IupSetfAttribute(statBar, "TITLE", "Ln %d, Col %d", line, col);
    return IUP_DEFAULT;
}

int exitProg(Ihandle *itemExit){

    if (g_config)
    {      
        if (!saveCheck(itemExit))
        {
            return IUP_IGNORE;
        }
         
        IupConfigDialogClosed(g_config, IupGetDialog(itemExit), "MainWindow");
        IupConfigSave(g_config);
        IupDestroy(g_config);
    }
    else
    {
        printf("[DEBUG] g_config is NULL\n");
    }

    return IUP_CLOSE;
}


// int debug(Ihandle *ih, int c){
//     printf("Pressed %d (%c)\n",c ,c);
//     return IUP_CONTINUE;
// }



int main(int argc, char **argv){
    Ihandle *box;
    Ihandle *fileMenu, *itemNew ,*itemSave, *itemSaveAs, *itemOpen, *itemRecent, *itemRevert, *itemExit;
    Ihandle *editMenu, *itemCut, *itemCopy, *itemPaste, *itemDelete, *itemSelAll, *itemGoto, *itemFind;
    Ihandle *formatMenu, *itemFont;
    Ihandle *aboutMenu, *itemAbout;
    Ihandle *subMenuFile, *subMenuEdit, *subMenuFormat, *subMenuAbout, *menu;
    Ihandle *statusBar;
    Ihandle *config;

    const char *font;

    IupOpen(&argc, &argv);

    
    textBox = IupText(NULL);
    IupSetAttribute(textBox, "MULTILINE", "YES");
    IupSetAttribute(textBox, "EXPAND", "YES");
    IupSetAttribute(textBox, "FONT", "Courier, 18");
    IupSetCallback(textBox, "CARET_CB", (Icallback)textBoxPointer);
    IupSetCallback(textBox, "VALUECHANGED_CB", (Icallback)textBoxChanged);
    IupSetCallback(textBox, "DROPFILES_CB", (Icallback)dropFile);

    statusBar = IupLabel("Ln 1, Col 1");
    IupSetAttribute(statusBar, "NAME", "STATUSBAR");
    IupSetAttribute(statusBar, "EXPAND", "HORIZONTAL");
    IupSetAttribute(statusBar, "PADDING", "10x5");


    itemNew = IupItem("New\t Ctrl+N", NULL);
    itemSave = IupItem("Save\t Ctrl+S", NULL);
    itemSaveAs = IupItem("Save As\t Ctrl+Shift+S", NULL);
    itemOpen = IupItem("Open\t Ctrl+O", NULL);
    itemRevert = IupItem("Revert", NULL);
    itemExit = IupItem("Exit\t Alt+4", NULL);


    itemCut = IupItem("Cut\t Ctrl+X", NULL);
    IupSetAttribute(itemCut, "NAME", "ITEM_CUT");
    itemCopy = IupItem("Copy\t Ctrl+C", NULL);
    IupSetAttribute(itemCopy, "NAME", "ITEM_COPY");
    itemPaste = IupItem("Paste\t Ctrl+P", NULL);
    IupSetAttribute(itemPaste, "NAME", "ITEM_PASTE");
    itemDelete = IupItem("Delete\t Del", NULL);
    IupSetAttribute(itemDelete, "NAME", "ITEM_DEL");
    itemSelAll = IupItem("Select All\t Ctrl+A", NULL);


    itemGoto = IupItem("Goto\t Ctrl+G", NULL);
    itemFind = IupItem("Find\t Ctrl+F", NULL);

    itemFont = IupItem("Change Font", NULL);
    itemAbout = IupItem("About", NULL);

    
    IupSetCallback(itemNew, "ACTION", (Icallback)newFileCallback);
    IupSetCallback(itemExit, "ACTION", (Icallback)exitProg);
    IupSetCallback(itemSave, "ACTION", (Icallback)save);
    IupSetCallback(itemSaveAs, "ACTION", (Icallback)saveAs);
    IupSetCallback(itemOpen, "ACTION", (Icallback)openFile);
    IupSetCallback(itemRevert, "ACTION", (Icallback)revertAct);

    IupSetCallback(itemGoto, "ACTION", (Icallback)gotoMainCallback);
    IupSetCallback(itemFind, "ACTION", (Icallback)findMainCallback);
    IupSetCallback(itemCut, "ACTION", (Icallback)editCutCallback);
    IupSetCallback(itemCopy, "ACTION", (Icallback)editCopyCallback);
    IupSetCallback(itemPaste, "ACTION", (Icallback)editPasteCallback);
    IupSetCallback(itemDelete, "ACTION", (Icallback)editDeleteCallback);
    IupSetCallback(itemSelAll, "ACTION", (Icallback)editSelectAllCallback);

    IupSetCallback(itemFont, "ACTION", (Icallback)modFont);

    IupSetCallback(itemAbout, "ACTION", (Icallback)showAbout);
 

    itemRecent = IupMenu(NULL);

    fileMenu = IupMenu(
        itemNew,
        itemSave,
        itemSaveAs,
        itemOpen,
        itemRevert,
        IupSeparator(),
        IupSubmenu("Recent Files", itemRecent),
        IupSeparator(),
        itemExit,
        NULL
    );

    editMenu = IupMenu(
        itemCut,
        itemCopy,
        itemPaste,
        itemDelete,
        IupSeparator(),
        itemSelAll,
        IupSeparator(),
        itemGoto,
        itemFind,
        NULL
    );

    formatMenu = IupMenu(
        itemFont,
        NULL
    );
    aboutMenu = IupMenu(
        itemAbout,
        NULL
    );

    subMenuFile = IupSubmenu("File", fileMenu);
    subMenuEdit = IupSubmenu("Edit", editMenu);
    subMenuFormat = IupSubmenu("Format", formatMenu);
    subMenuAbout = IupSubmenu("About", aboutMenu);

    menu = IupMenu(
        subMenuFile,
        subMenuEdit,
        subMenuFormat,
        subMenuAbout,
        NULL
    );

    box = IupVbox(
        textBox,
        statusBar,
        NULL
    );
    
    diagBox = IupDialog(box);
    IupSetAttributeHandle(diagBox, "MENU", menu);
    IupSetAttribute(diagBox, "TITLE", "Notepad");
    IupSetAttribute(diagBox, "SIZE", "HALFxHALF");
    IupSetCallback(diagBox, "CLOSE_CB", (Icallback)exitProg);
    IupSetCallback(diagBox, "DROPFILES_CB", (Icallback)dropFile);
    IupSetCallback(editMenu, "OPEN_CB", (Icallback)editMenuCallback);
    IupSetCallback(fileMenu, "OPEN_CB", (Icallback)fileMenuCallback);


    g_config = IupConfig();
    IupSetStrAttribute(g_config, "APP_NAME", "Notepad");
    IupConfigLoad(g_config);

    IupSetAttributeHandle(diagBox, "CONFIG", g_config);

    font = IupConfigGetVariableStr(g_config, "MainWindow", "Font");
    if (font)
    {
        IupSetStrAttribute(textBox, "FONT", font);
    }
    


    IupSetAttributeHandle(NULL, "PARENTDIALOG", diagBox);

    IupSetCallback(diagBox, "K_cN", (Icallback)newFileCallback);
    IupSetCallback(diagBox, "K_cO", (Icallback)openFile);
    IupSetCallback(diagBox, "K_cS", (Icallback)save);
    IupSetCallback(diagBox, "K_cyS", (Icallback)saveAs);
    IupSetCallback(textBox, "K_cyS", (Icallback)saveAs);
    // IupSetCallback(diagBox, "K_ANY", (Icallback)debug);
    IupSetCallback(diagBox, "K_cG", (Icallback)gotoMainCallback);
    IupSetCallback(diagBox, "K_cF", (Icallback)findMainCallback);


    IupConfigRecentInit(g_config, itemRecent, configCallback, 10);

    IupConfigDialogShow(g_config, diagBox, "MainWindow");

    
    // init new file

    newFile(diagBox);

    // for opening file from cmd

    if (argc > 1 && argv[1])
    {
        const char *fileName = argv[1];
        openSubFunction(diagBox,fileName);
    }
    

    IupMainLoop();

    IupClose();
    return EXIT_SUCCESS;

}