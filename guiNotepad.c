#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iup.h>
#include <iup_config.h>

Ihandle *textBox = NULL;
char filePath[2048] = "";

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

void writeFile(const char *fileName, const char *str, int count){

    FILE *fp = fopen(fileName, "w");
    if(!fp){
        IupMessagef("Error", "Cannot Open File: %s", fileName);
    }

    fwrite(str, 1, count, fp); // writes file

    if(ferror(fp)){
        IupMessagef("Error", "Failed Writing into File: %s", fileName);
    }

    fclose(fp);
    
}

//   -------    CALLBACKS  -------

int openFile(void){
    Ihandle *fileDiag = IupFileDlg(); // opens built in file selector
    IupSetStrAttribute(fileDiag, "DIALOGTYPE", "OPEN");
    IupSetAttribute(fileDiag, "EXTILTER", "Text Files|*.txt|All Files|*.*|");

    IupPopup(fileDiag, IUP_CENTER, IUP_CENTER);

    // check if user cancels
    if (IupGetInt(fileDiag, "STATUS") != -1){
        char *fileName = IupGetAttribute(fileDiag, "VALUE");
        char *str = readFile(fileName);
        if (str){
            IupSetStrAttribute(textBox, "VALUE", str);

            free(str);
            
        }
        strcpy(filePath, fileName);
    }

    IupDestroy(fileDiag);
    return IUP_DEFAULT;
}


int saveAs(void){
    Ihandle *fileDiag = IupFileDlg(); // opens built in file selector
    IupSetStrAttribute(fileDiag, "DIALOGTYPE", "SAVE");
    IupSetAttribute(fileDiag, "EXTILTER", "Text Files|*.txt|All Files|*.*|");

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

        char *str = IupGetAttribute(textBox, "VALUE");
        int count = IupGetInt(textBox, "COUNT");
        writeFile(fileName, str, count);
        strcpy(filePath, fileName);
    }

    IupDestroy(fileDiag);
    return IUP_DEFAULT;
}

int save(void){
    if (strlen(filePath) == 0)
    {
        return saveAs();
    }
    
    char *str = IupGetAttribute(textBox, "VALUE");
    int count = strlen(str);

    writeFile(filePath, str, count);

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

int exitProg(void){
    return IUP_CLOSE;
}

// int debug(Ihandle *ih, int c){
//     printf("Pressed %d (%c)\n",c ,c);
//     return IUP_CONTINUE;
// }



int main(int argc, char **argv){
    Ihandle *diagBox, *box;
    Ihandle *fileMenu, *itemSave, *itemSaveAs, *itemOpen, *itemExit;
    Ihandle *searchMenu, *itemGoto, *itemFind;
    Ihandle *formatMenu, *itemFont;
    Ihandle *aboutMenu, *itemAbout;
    Ihandle *subMenuFile, *subMenuSearch, *subMenuFormat, *subMenuAbout, *menu;
    Ihandle *statusBar;

    IupOpen(&argc, &argv);


    textBox = IupText(NULL);
    IupSetAttribute(textBox, "MULTILINE", "YES");
    IupSetAttribute(textBox, "EXPAND", "YES");

    statusBar = IupLabel("Ln 1, Col 1");
    IupSetAttribute(statusBar, "NAME", "STATUSBAR");
    IupSetAttribute(statusBar, "EXPAND", "HORIZONTAL");
    IupSetAttribute(statusBar, "PADDING", "10x5");

    itemSave = IupItem("Save\t Ctrl+S", NULL);
    itemSaveAs = IupItem("Save As\t Ctrl+Shift+S", NULL);
    itemOpen = IupItem("Open\t Ctrl+O", NULL);
    itemExit = IupItem("Exit\t Alt+4", NULL);
    itemGoto = IupItem("Goto\t Ctrl+G", NULL);
    itemFind = IupItem("Find\t Ctrl+F", NULL);
    itemFont = IupItem("Change Font", NULL);
    itemAbout = IupItem("About", NULL);

    IupSetCallback(itemExit, "ACTION", (Icallback)exitProg);
    IupSetCallback(itemSave, "ACTION", (Icallback)save);
    IupSetCallback(itemSaveAs, "ACTION", (Icallback)saveAs);
    IupSetCallback(itemOpen, "ACTION", (Icallback)openFile);
    IupSetCallback(itemGoto, "ACTION", (Icallback)gotoMainCallback);
    IupSetCallback(itemFind, "ACTION", (Icallback)findMainCallback);
    IupSetCallback(itemFont, "ACTION", (Icallback)modFont);
    IupSetCallback(itemAbout, "ACTION", (Icallback)showAbout);
    IupSetCallback(textBox, "CARET_CB", (Icallback)textBoxPointer);

    fileMenu = IupMenu(
        itemSave,
        itemSaveAs,
        itemOpen,
        IupSeparator(),
        itemExit,
        NULL
    );

    searchMenu = IupMenu(
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
    subMenuSearch = IupSubmenu("Search", searchMenu);
    subMenuFormat = IupSubmenu("Format", formatMenu);
    subMenuAbout = IupSubmenu("About", aboutMenu);

    menu = IupMenu(
        subMenuFile,
        subMenuSearch,
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

    IupSetAttributeHandle(NULL, "PARENTDIALOG", diagBox);

    IupSetCallback(diagBox, "K_cO", (Icallback)openFile);
    IupSetCallback(diagBox, "K_cS", (Icallback)save);
    IupSetCallback(diagBox, "K_cyS", (Icallback)saveAs);
    IupSetCallback(textBox, "K_cyS", (Icallback)saveAs);
    // IupSetCallback(diagBox, "K_ANY", (Icallback)debug);
    IupSetCallback(diagBox, "K_cG", (Icallback)gotoMainCallback);
    IupSetCallback(diagBox, "K_cF", (Icallback)findMainCallback);

    IupShowXY(diagBox, IUP_CENTERPARENT, IUP_CENTERPARENT);
    IupSetAttribute(diagBox, "USERSIZE", NULL);


    IupMainLoop();

    IupClose();
    return EXIT_SUCCESS;

}