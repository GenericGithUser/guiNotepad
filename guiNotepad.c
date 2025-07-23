#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iup.h>

Ihandle *textBox = NULL;
char filePath[2048] = "";

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

int textBoxPointer(Ihandle *ih, int line, int col){
    Ihandle *statBar = IupGetDialogChild(ih, "STATUSBAR");
    IupSetfAttribute(statBar, "TITLE", "Ln %d, Col %d", line, col);
    return IUP_DEFAULT;
}

int exitProg(void){
    return IUP_CLOSE;
}

int main(int argc, char **argv){
    Ihandle *diagBox, *box;
    Ihandle *fileMenu, *itemSave, *itemSaveAs, *itemOpen, *itemExit;
    Ihandle *formatMenu, *itemFont;
    Ihandle *aboutMenu, *itemAbout;
    Ihandle *subMenuFile, *subMenuFormat, *subMenuAbout, *menu;
    Ihandle *statusBar;

    IupOpen(&argc, &argv);


    textBox = IupText(NULL);
    IupSetAttribute(textBox, "MULTILINE", "YES");
    IupSetAttribute(textBox, "EXPAND", "YES");

    statusBar = IupLabel("Ln 1, Col 1");
    IupSetAttribute(statusBar, "NAME", "STATUSBAR");
    IupSetAttribute(statusBar, "EXPAND", "HORIZONTAL");
    IupSetAttribute(statusBar, "PADDING", "10x5");

    itemSave = IupItem("Save", NULL);
    itemSaveAs = IupItem("Save As", NULL);
    itemOpen = IupItem("Open", NULL);
    itemExit = IupItem("Exit", NULL);
    itemFont = IupItem("Change Font", NULL);
    itemAbout = IupItem("About", NULL);

    IupSetCallback(itemExit, "ACTION", (Icallback)exitProg);
    IupSetCallback(itemSave, "ACTION", (Icallback)save);
    IupSetCallback(itemSaveAs, "ACTION", (Icallback)saveAs);
    IupSetCallback(itemOpen, "ACTION", (Icallback)openFile);
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
    formatMenu = IupMenu(
        itemFont,
        NULL
    );
    aboutMenu = IupMenu(
        itemAbout,
        NULL
    );

    subMenuFile = IupSubmenu("File", fileMenu);
    subMenuFormat = IupSubmenu("Format", formatMenu);
    subMenuAbout = IupSubmenu("About", aboutMenu);

    menu = IupMenu(
        subMenuFile,
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
    IupSetAttribute(diagBox, "TITLE", "testNotepad");
    IupSetAttribute(diagBox, "SIZE", "HALFxHALF");

    IupShowXY(diagBox, IUP_CENTER, IUP_CENTER);
    IupSetAttribute(diagBox, "USERSIZE", NULL);

    IupMainLoop();

    IupClose();
    return EXIT_SUCCESS;

}