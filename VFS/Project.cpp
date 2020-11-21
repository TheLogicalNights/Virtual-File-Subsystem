/*
    Project Name : Customised Virtual File subsystem.
*/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>

#define MAX_INODE 100
#define MAX_FILE_SIZE 1024
#define REGULAR 1
#define TRUE 1
#define FALSE 0
#define READ 4
#define WRITE 2
#define SET 1
#define CUR 2
#define END 3
typedef int BOOL;

typedef struct inode
{
    char *filename;
    int Inode_No;
    int FileSize;
    int ActualFileSize;
    int FileType;
    char *Buffer;
    int LinkCount;
    int ReferenceCount;
    int Permission;
    struct inode * next;
}INODE,*PINODE,**PPINODE;

struct SuperBlock
{
    int No_of_Nodes;
    int No_of_free_Nodes;
}SuperObj;

typedef struct FileTable
{
    int ReadOfset;
    int WriteOfset;
    int Mode;
    int count;
    PINODE inodeptr;
}File,*PFile;

struct ufdt
{
    PFile ufdt[MAX_INODE];
}ufdtobj;

struct OpenFiles
{
    char filename[MAX_INODE][80];
    int fd[MAX_INODE];
}openfilesobj;

PINODE Head = NULL;

void InitOpenFiles()
{
    int i = 0;
    for(i=0;i<MAX_INODE;i++)
    {
        openfilesobj.fd[i] = 0;
    }
}
void CreateUFDT()
{
    int i = 0;

    while(i<MAX_INODE)
    {
        ufdtobj.ufdt[i] = NULL;
        i++;
    }
}
void CreateSuperBlock()
{
    SuperObj.No_of_Nodes = MAX_INODE;
    SuperObj.No_of_free_Nodes = MAX_INODE;
}
void CreateDILB()
{
    int i = 0;
    PINODE newnode = NULL;
    PINODE Temp = Head;
    while(i < MAX_INODE)
    {
        newnode = (PINODE)malloc(sizeof(INODE));
        newnode->Inode_No = i;
        newnode->FileSize = MAX_FILE_SIZE;
        newnode->ActualFileSize = 0;
        newnode->FileType = 0;
        newnode->LinkCount = 0;
        newnode->ReferenceCount = 0;
        newnode->Permission = 0;
        newnode->Buffer = NULL;
        newnode->next = NULL;

        if(Head==NULL)
        {
            Head  = newnode;
            Temp = Head;
        }
        else
        {
            Temp->next = newnode;
            Temp=Temp->next;
        }
        i++;
    }
}
void DisplayHelp()
{
    printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n\n");
    printf("create : To create new files\n");
    printf("ls : To List out all files\n");
    printf("clear : To clear console\n");
    printf("open : To open existing file");
    printf("close : To close already opened file\n");
    printf("closeall : To close all opened files\n");
    printf("read : To read opened file\n");
    printf("write : To write data into opened file\n");
    printf("exit : To exit the project\n");
    printf("stat : To display all the information of the file using filename\n");
    printf("fstat : To display all the information of the file using file descriptor\n");
    printf("truncate : To truncate the file to perticular size\n");
    printf("rm : To remove the file\n");
    printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
}
void DisplayMan(char *name)
{
    printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n\n");

    if(name == NULL) 
    {
        return;
    }
    if(strcmp(name,"create") == 0)
    {
        printf("Description : Used to create new regular file\n");
        printf("Usage : create File_name Permission\n");
    }
    else if(strcmp(name,"read") == 0)
    {
        printf("Description : Used to read opened regular file\n");
        printf("Usage : read File_name\n");
    }
    else if(strcmp(name,"write") == 0)
    {
        printf("Description : Used to write data into opened regular file\n");
        printf("Usage : write File_name\n");
        printf("        Write -a File_name to append\n");   
    }
    else if(strcmp(name,"ls") == 0)
    {
        printf("Description : Used to list all the files\n");
        printf("Usage : ls\n");
        printf("        ls -i for list with inodes\n");
        printf("        ls -i for list of open files\n");
    }
    else if(strcmp(name,"stat") == 0)
    {
        printf("Description : Used to display information of file using filename\n");
        printf("Usage : stat File_name\n");
    }
    else if(strcmp(name,"fstat") == 0)
    {
        printf("Description : Used to display information of file using file descriptor\n");
        printf("Usage : fstat File_descriptor\n");
    }
    else if(strcmp(name,"truncate") == 0)
    {
        printf("Description : Used to truncate the file to the perticulat number of bytes\n");
        printf("Usage : truncate No_of_bytes\n");
    }
    else if(strcmp(name,"open") == 0)
    {
        printf("Description : Used to open existing file\n");
        printf("Usage : open File_name Mode\n");
    }
    else if(strcmp(name,"close") == 0)
    {
        printf("Description : Used to close already opened file\n");
        printf("Usage : close File_name\n");
    }
    else if(strcmp(name,"closeall") == 0)
    {
        printf("Description : Used to close all opened file\n");
        printf("Usage : closeall\n");
    }
    else if(strcmp(name,"lseek") == 0)
    {
        printf("Description : Used to change ofset of file\n");
        printf("Usage : lseek No_of_bytes Position File_name\n");
    }
    else if(strcmp(name,"rm") == 0)
    {
        printf("Description : Used to remove the file\n");
        printf("Usage : rm file_name\n");
    }
    else
    {
        printf("ERROR : man page not available.\n");
    }

    printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
}
int ChkUFDT()
{
    int i = 0;
    for(i=0;i<MAX_INODE;i++)
    {
        if(ufdtobj.ufdt[i]==NULL)
        {
            break;
        }
    }
    if(i==MAX_INODE)
    {
        return -1;
    }
    else
    {
        return i;
    }
}
int ChkInode()
{
    PINODE Temp = Head;
    int i = 0;
    while(Temp != NULL)
    {
        if(Temp->FileType==0)
        {
            break;
        }
        Temp = Temp->next;
        i++;
    }
    if(Temp==NULL)
    {
        return -1;
    }
    else
    {
        return i;
    }
}

BOOL ChkFile(char *filename)
{
    PINODE Temp = Head;
    while(Temp!=NULL)
    {
        if(Temp->FileType!=0)
        {
            if(strcmp(Temp->filename,filename)==0)
            {
                break;
            }
        }
        Temp = Temp->next;
    }
    if(Temp==NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int CreateFile(char *filename,int Permission)
{
    
    int FreeInode = 0;
    int FreeUFDT = 0;
    
    if((filename==NULL) || ((Permission != READ) && (Permission != WRITE) && (Permission != (READ+WRITE))))
    {
        return -1;
    }
    if(SuperObj.No_of_free_Nodes==0)
    {
        return -2;
    }
    
    FreeInode = ChkInode();

    FreeUFDT = ChkUFDT();
    if(FreeUFDT==-1)
    {
        return -4;
    }
    BOOL bRet = ChkFile(filename);
    if(bRet==FALSE)
    {
        return -3;
    }           

    ufdtobj.ufdt[FreeUFDT] = (PFile)malloc(sizeof(File));
    ufdtobj.ufdt[FreeUFDT]->ReadOfset = 0;
    ufdtobj.ufdt[FreeUFDT]->WriteOfset = 0;
    ufdtobj.ufdt[FreeUFDT]->count = 1;
    ufdtobj.ufdt[FreeUFDT]->Mode = Permission;
    
    PINODE Temp = Head;
    while(Temp != NULL)
    {
        if(Temp->FileType==0)
        {
            break;
        }
        Temp = Temp->next;
    }
    ufdtobj.ufdt[FreeUFDT]->inodeptr = Temp;

    ufdtobj.ufdt[FreeInode]->inodeptr->filename = (char *)malloc(80);
    strcpy(ufdtobj.ufdt[FreeInode]->inodeptr->filename,filename);
    ufdtobj.ufdt[FreeInode]->inodeptr->FileSize = MAX_FILE_SIZE;
    ufdtobj.ufdt[FreeInode]->inodeptr->ActualFileSize = 0;
    ufdtobj.ufdt[FreeInode]->inodeptr->FileType = REGULAR;
    ufdtobj.ufdt[FreeInode]->inodeptr->LinkCount = 1;
    ufdtobj.ufdt[FreeInode]->inodeptr->ReferenceCount = 0;
    ufdtobj.ufdt[FreeInode]->inodeptr->Buffer = (char *)malloc(1024);
    ufdtobj.ufdt[FreeInode]->inodeptr->Permission = Permission;

    SuperObj.No_of_free_Nodes -= 1;

    return FreeUFDT;
}
BOOL RemoveFile(char *filename)
{
    int i = 0;
    for(i=0;i< MAX_INODE ;i++)
    {
        if(ufdtobj.ufdt[i]!=NULL)
        {
            if(strcmp(ufdtobj.ufdt[i]->inodeptr->filename,filename)==0)
            {
                free(ufdtobj.ufdt[i]->inodeptr->filename);
                ufdtobj.ufdt[i]->inodeptr->FileSize = MAX_FILE_SIZE;
                ufdtobj.ufdt[i]->inodeptr->ActualFileSize = 0;
                ufdtobj.ufdt[i]->inodeptr->FileType = 0;
                ufdtobj.ufdt[i]->inodeptr->LinkCount = 0;
                ufdtobj.ufdt[i]->inodeptr->ReferenceCount = 0;
                ufdtobj.ufdt[i]->inodeptr->Permission = 0;
                ufdtobj.ufdt[i]->inodeptr->Buffer = NULL;
                free(ufdtobj.ufdt[i]->inodeptr->Buffer);

                ufdtobj.ufdt[i]->ReadOfset = 0;
                ufdtobj.ufdt[i]->WriteOfset = 0;
                ufdtobj.ufdt[i]->count = 1;
                ufdtobj.ufdt[i]->Mode = 0;
                ufdtobj.ufdt[i]->inodeptr = NULL;

                free(ufdtobj.ufdt[i]);
                ufdtobj.ufdt[i] = NULL;

                SuperObj.No_of_free_Nodes += 1;

                break;
            }
        }
    }
    if(i==MAX_INODE)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
void LsFile()
{
    PINODE Temp = Head;
    while(Temp!=NULL)
    {
        if(Temp->FileType==1)
        {
            printf("%s\n",Temp->filename);
        }
        Temp = Temp->next;
    }
}
void LsInodes()
{
    
    PINODE Temp = Head;
    
    while(Temp != NULL)
    {
        if(Temp->FileType==1)
        {
            printf("%s  %d\n",Temp->filename,Temp->Inode_No);
        }
        Temp = Temp->next;
    }
}
void SetEnvi()
{
    CreateDILB();
    CreateSuperBlock();
    CreateUFDT();
    InitOpenFiles();
    printf("All set....\n");
}

void DisplayStat(char *filename)
{
    PINODE Temp = Head;

    while(Temp!=NULL)
    {
        if(Temp->FileType!=0)
        {
            if(strcmp(Temp->filename,filename)==0)
            {
                printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n\n");
                printf("File Name : %s\n",Temp->filename);
                printf("Inode number : %d\n",Temp->Inode_No);
                printf("File Size : %d\n",Temp->FileSize);
                printf("Actual File Size : %d\n",Temp->ActualFileSize);
                if(Temp->FileType==1)
                {
                    printf("File Type : Regular\n");
                } 
                printf("Link Count : %d\n",Temp->LinkCount);
                printf("Reference Count : %d\n",Temp->ReferenceCount);
                printf("Permission : %d\n",Temp->Permission);
                printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
                break;
            }
        }
        Temp = Temp->next;
    }
    if(Temp==NULL)
    {
        printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n\n");
        printf("There is no such file.....\n");
        printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
        return;
    }   
}
void DisplayStatX(int iFd)
{
    if(ufdtobj.ufdt[iFd]==NULL)
    {
        printf("Error : There is no such file descriptor assigned...\n");
        return;
    }
    printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n\n");
    printf("File Name : %s\n",ufdtobj.ufdt[iFd]->inodeptr->filename);
    printf("Inode number : %d\n",ufdtobj.ufdt[iFd]->inodeptr->Inode_No);
    printf("File Size : %d\n",ufdtobj.ufdt[iFd]->inodeptr->FileSize);
    printf("Actual File Size : %d\n",ufdtobj.ufdt[iFd]->inodeptr->ActualFileSize);
    if(ufdtobj.ufdt[iFd]->inodeptr->FileType==1)
    {
        printf("File Type : Regular\n");
    } 
    printf("Link Count : %d\n",ufdtobj.ufdt[iFd]->inodeptr->LinkCount);
    printf("Reference Count : %d\n",ufdtobj.ufdt[iFd]->inodeptr->ReferenceCount);
    printf("Permission : %d\n",ufdtobj.ufdt[iFd]->inodeptr->Permission);
    printf("\n+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
}
int OpenFile(char *filename,int Permission)
{
    PINODE Temp = Head;
    int i = 0;
    int j = 0;

    if(filename==NULL || ((Permission != READ) && (Permission != WRITE) && (Permission != (READ+WRITE))))
    {
        return -1;
    }
    while(Temp != NULL)
    {
        if(Temp->FileType==1)
        {
            if(strcmp(Temp->filename,filename)==0)
            {
                break;
            }
        }
        Temp = Temp->next;
    }
    if(Temp==NULL)
    {
        return -2;
    }
    for(i=0;i<MAX_INODE;i++)
    {
        if(ufdtobj.ufdt[i] == NULL)
        {
            break;
        }
    }
    if(i==MAX_INODE)
    {
        return -3;
    }

    if((Temp->Permission==READ) && ((Permission == WRITE) || (Permission==READ+WRITE)))
    {
        return -3;
    }
    if((Temp->Permission==WRITE) && ((Permission == READ) || (Permission==READ+WRITE)))
    {
        return -3;
    }

    ufdtobj.ufdt[i] = (PFile)malloc(sizeof(File));
    ufdtobj.ufdt[i]->count = 1;
    ufdtobj.ufdt[i]->Mode = Permission;
    if(Permission==READ)
    {
        ufdtobj.ufdt[i]->ReadOfset = 0;
    }
    if(Permission==WRITE)
    {
        ufdtobj.ufdt[i]->WriteOfset = 0;
    }
    else
    {
        ufdtobj.ufdt[i]->ReadOfset = 0;
        ufdtobj.ufdt[i]->WriteOfset = 0;
    }

    ufdtobj.ufdt[i]->inodeptr = Temp;
    (ufdtobj.ufdt[i]->inodeptr->ReferenceCount)++; 

    for(j=0;j<MAX_INODE;j++)
    {
        if(openfilesobj.fd[j]==0)
        {
            break;
        }
    }
    strcpy(openfilesobj.filename[j],filename);
    openfilesobj.fd[j] = i;

    return i;
}
int CloseFile(char *filename)
{
    int i = 0;
    if(filename==NULL)
    {
        return -1;
    }
    for(i=0;i<MAX_INODE;i++)
    {
        if(openfilesobj.fd[i]!=0)
        {
            if(strcmp(openfilesobj.filename[i],filename)==0)
            {
                ufdtobj.ufdt[openfilesobj.fd[i]]->count = 0;
                ufdtobj.ufdt[openfilesobj.fd[i]]->Mode = 0;
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ReferenceCount)--;
                ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr = NULL;
                ufdtobj.ufdt[openfilesobj.fd[i]] = NULL;
                openfilesobj.fd[i] = 0;
                break;
            }
        }
    }
    if(i==MAX_INODE)
    {
        return -2;
    }

    return 0;
}
int CloseFileX(int fd)
{
    int i = 0;
    
    for(i=0;i<MAX_INODE;i++)
    {
        if(openfilesobj.fd[i] != 0)
        {
            if(openfilesobj.fd[i]==fd)
            {
                ufdtobj.ufdt[openfilesobj.fd[i]]->count = 0;
                ufdtobj.ufdt[openfilesobj.fd[i]]->Mode = 0;
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset = 0;
                ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ReferenceCount)--;
                ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr = NULL;
                ufdtobj.ufdt[openfilesobj.fd[i]] = NULL;
                openfilesobj.fd[i] = 0;
                break;
            }
        }
    }
    if(i==MAX_INODE)
    {
        return -2;
    }

    return 0;
}
int GetFdByName(char *filename)
{
    int i = 0;

    for(i=0;i<MAX_INODE;i++)
    {
        if(ufdtobj.ufdt[i] != NULL)
        {
            if(ufdtobj.ufdt[i]->inodeptr->FileType==1)
            {
                if(strcmp(ufdtobj.ufdt[i]->inodeptr->filename,filename)==0)
                {
                    break;
                }
            }
        }
    }
    if(i==MAX_INODE)
    {
        return -1;
    }
    else
    {
        return i;
    }
}
void ListOpen()
{
    for(int i=0;i<MAX_INODE;i++)
    {
        if(openfilesobj.fd[i]>0)
        {
            printf("%s\t",openfilesobj.filename[i]);
            printf("%d\n",openfilesobj.fd[i]);
        }
    }
}

int WriteData(char *filename,char *data)
{
    if(filename==NULL)
    {
        return -1;
    }
    int i = 0;
    for(i=0;i<MAX_INODE;i++)
    {
        if(openfilesobj.fd[i] != 0)
        {
            if(strcmp(openfilesobj.filename[i],filename)==0)
            {
                break;
            }
        }
    }
    if(i==MAX_INODE)
    {
        return -3;
    }
    if(ufdtobj.ufdt[openfilesobj.fd[i]]->Mode == READ)
    {
        return -2;
    }
     
    strncpy((ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer),data,strlen(data));
    
    if((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
    {
        while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
        {
            (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
            (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
        }
    }
    else if((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
    {
        while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
        {
            (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
            (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
        }
    }

    ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize = strlen((ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer));
    
    return 0;
}
void ReadFile(char *filename)
{
    int i = 0;

    for(i=0;i<MAX_INODE;i++)
    {
        if(openfilesobj.fd[i] != 0)
        {
            if(strcmp(openfilesobj.filename[i],filename)==0)
            {
                break;
            }
        }
    }
    if(i==MAX_INODE)
    {
        return ;
    }
    if(ufdtobj.ufdt[openfilesobj.fd[i]]->Mode == WRITE)
    {
        return ;
    }
    printf("%s\n",ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer);
}
int AppendFile(char *filename,char *data)
{
    if(filename==NULL)
    {
        return -1;
    }
    int i = 0;
    for(i=0;i<MAX_INODE;i++)
    {
        if(openfilesobj.fd[i] != 0)
        {
            if(strcmp(openfilesobj.filename[i],filename)==0)
            {
                break;
            }
        }
    }
    if(i==MAX_INODE)
    {
        return -3;
    }
    if(ufdtobj.ufdt[openfilesobj.fd[i]]->Mode == READ)
    {
        return -2;
    }

    if((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
    {
        while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
        {
            (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
            (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
        }
    }
    else if((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
    {
        while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
        {
            (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
            (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
        }
    }

    strncpy((ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer)+(ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize),data,strlen(data));

    ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize = strlen((ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer));
    
    return 0;
}
int LseekFile(int iLen,int iPos,char *filename)
{
    if((filename==NULL) || (iPos>3))
    {
        return -1;
    }
    int i = 0;
    for(i=0;i<MAX_INODE;i++)
    {
        if(openfilesobj.fd[i] != 0)
        {
            if(strcmp(openfilesobj.filename[i],filename)==0)
            {
                break;
            }
        }
    }
    if(i==MAX_INODE)
    {
        return -3;
    }
    if(((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)+iLen) >= (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize) || ((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)+iLen) < 0)
    {
        return -2;        
    }
    if(ufdtobj.ufdt[openfilesobj.fd[i]]->Mode==READ)
    {
        if(iPos==SET)
        {
            if(iLen!=0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen==0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
            }
        }
        else if(iPos==CUR)
        {
            if(iLen==0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen>0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen<0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
        }
        else if(iPos==END)
        {
            if(iLen==0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            if(iLen>0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen<0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
        }
    }
    else if(ufdtobj.ufdt[openfilesobj.fd[i]]->Mode==WRITE)
    {
        if(iPos==SET)
        {
            if(iLen!=0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
            }
            else if(iLen==0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
                }
            }
        }
        else if(iPos==CUR)
        {
            if(iLen==0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
            }
            else if(iLen>0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
            }
            else if(iLen<0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
            }
        }
        else if(iPos==END)
        {
            if(iLen==0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
            }
            if(iLen>0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
            }
            else if(iLen<0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset);
            }
        }
    }
    else if(ufdtobj.ufdt[openfilesobj.fd[i]]->Mode==READ+WRITE)
    {
        if(iPos==SET)
        {
            if(iLen!=0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen==0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }

                while((ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->WriteOfset)--;
                }
            }
        }
        else if(iPos==CUR)
        {
            if(iLen==0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen>0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen<0)
            {
                int Current_Offset = (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                while(Current_Offset != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    Current_Offset--;
                }
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen; 
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
        }
        else if(iPos==END)
        {
            if(iLen==0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            if(iLen>0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
            else if(iLen<0)
            {
                while((ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) != 0)
                {
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) -= 1;
                    (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset)--;
                }
                ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset = 0;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->ActualFileSize)-1;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset) += iLen;
                (ufdtobj.ufdt[openfilesobj.fd[i]]->inodeptr->Buffer) += (ufdtobj.ufdt[openfilesobj.fd[i]]->ReadOfset);
            }
        }
    }   
    return 0; 
}
int main()
{
    char str[80],command[4][80];
    int iCount = 0;
    char data[MAX_FILE_SIZE] = {'\0'};

    SetEnvi();
    
    while(1)
    {
        printf("Swapnil's VFS : > ");
        scanf(" %[^'\n']s",str);

        iCount = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);
    
        if(iCount == 1)
        {
            if(strcmp(command[0],"help")==0)
            {
                DisplayHelp();
                continue;
            }
            else if(strcmp(command[0],"ls")==0)
            {
                LsFile();
            }
            else if(strcmp(command[0],"clear")==0)
            {
                system("clear");
                continue;
            }
            else if(strcmp(command[0],"exit")==0)
            {
                printf("Thank you for using our application..\n");
                break;
            }
            else
            {
                printf("Error : %s Command not found !!\nUse \"help\" for help\n",str);
                continue;
            }
            
        }
        else if(iCount == 2)
        {
            if(strcmp(command[0],"man")==0)
            {
                DisplayMan(command[1]);
            }
            else if(strcmp(command[0],"stat")==0)
            {   
                DisplayStat(command[1]);
            }
            else if(strcmp(command[0],"fstat")==0)
            {   
                DisplayStatX(atoi(command[1]));
            }
            else if(strcmp(command[0],"rm")==0)
            {
                BOOL bRet = FALSE;
                bRet = RemoveFile(command[1]);
                if(bRet==TRUE)
                {
                    printf("%s : removed successfuly..\n",command[1]);
                }
                else
                {
                    printf("Error : there is no such file...\n");
                }
            }
            else if(strcmp(command[0],"fd")==0)
            {
                int iRet = GetFdByName(command[1]);
                if(iRet==-1)
                {
                    printf("There is no such file...\n");
                    continue;
                }
                printf("Fd of file %s is : %d\n",command[1],iRet);
            }
            else if(strcmp(command[0],"ls")==0)
            {
                if(strcmp(command[1],"-o")==0)
                {
                    ListOpen();
                }
                else if(strcmp(command[1],"-i")==0)
                {
                    LsInodes();
                }
            }
            else if(strcmp(command[0],"close")==0)
            {
                int iRet = CloseFile(command[1]);
                if(iRet==-1)
                {
                    printf("Error : Incorrect Parameters...\n");
                }
                else if(iRet==-2)
                {
                    printf("Error : There is no such file opened...\n");
                }
                else
                {
                    printf("%s Closed...\n",command[1]);
                }
            }
            else if(strcmp(command[0],"closefd")==0)
            {
                int iRet = CloseFileX(atoi(command[1]));
                if(iRet==-1)
                {
                    printf("Error : Incorrect Parameters...\n");
                }
                else if(iRet==-2)
                {
                    printf("Error : There is no such file opened...\n");
                }
                else
                {
                    printf("%s Closed...\n",command[1]);
                }
            }
            else if(strcmp(command[0],"write")==0)
            {
                printf("Enter data to write into file...\n");
                scanf(" %[^'\n']s",data);
                
                int iRet = WriteData(command[1],data);
                if(iRet==-1)
                {
                    printf("Error : Incorrect Parameters...\n");
                }
                else if(iRet==-2)
                {
                    printf("Error : Unable to write data into the file...\n");
                }
                else if(iRet==-3)
                {
                    printf("Error : There is no such file opened...\n");
                }
                else
                {
                    printf("Data gets successfully written into file...\n");
                }
            }
            else if(strcmp(command[0],"read")==0)
            {
                ReadFile(command[1]);
            }
            else
            {
                printf("Error : %s Command not found !!\nUse \"help\" for help\n",str);
                continue;
            }
            
        }
        else if(iCount == 3)
        {
            if(strcmp(command[0],"create")==0)
            {
                int iRet = CreateFile(command[1],atoi(command[2]));
                if(iRet>=0)
                {
                    printf("File created successfully with file descriptor : %d\n",iRet);
                }
                else if(iRet==-1)
                {
                    printf("ERROR : Incorrect parameters\n");
                }
                else if(iRet==-2)
                {
                    printf("ERROR : There is no inodes\n");
                }
                else if(iRet==-3)
                {
                    printf("ERROR : File already exists\n");
                }
                else if(iRet==-4)
                {
                    printf("ERROR : Memory allocation failure\n");
                }
            }
            else if(strcmp(command[0],"open")==0)
            {
                int iRet = OpenFile(command[1],atoi(command[2]));
                if(iRet>=0)
                {
                    printf("File opened successfully with file descriptor : %d\n",iRet);
                }
                else if(iRet==-1)
                {
                    printf("ERROR : Incorrect parameters\n");
                }
                else if(iRet==-2)
                {
                    printf("ERROR : There is no such file\n");
                }
                else if(iRet==-3)
                {
                    printf("ERROR : Unable to open file\n");
                }
            }
            else if(strcmp(command[0],"write")==0)
            {
                if(strcmp(command[1],"-a")==0)
                {
                    printf("Enter data to write into file...\n");
                    scanf(" %[^'\n']s",data);
                
                    int iRet = AppendFile(command[2],data);
                    if(iRet==-1)
                    {
                        printf("Error : Incorrect Parameters...\n");
                    }
                    else if(iRet==-2)
                    {
                        printf("Error : Unable to write data into the file...\n");
                    }
                    else if(iRet==-3)
                    {
                        printf("Error : There is no such file opened...\n");
                    }
                    else
                    {
                        printf("Data gets successfully written into file...\n");
                    }
                }
            }
            else
            {
                printf("Error : %s Command not found !!\nUse \"help\" for help\n",str);
                continue;
            }            
        }
        else if(iCount == 4)
        {
            if(strcmp(command[0],"lseek")==0)
            {
                int iRet = LseekFile(atoi(command[1]),atoi(command[2]),command[3]);
                if(iRet==-1)
                {
                    printf("Error : Incorrect Parameters...\n");
                }
                else if(iRet==-2)
                {
                    printf("Error : Unable to lseek the file...\n");
                }
                else if(iRet==-3)
                {
                    printf("Error : There is no such file opened...\n");
                }
                else
                {
                    printf("Offset of file changed successfully...\n");
                }
            }
            else
            {
                printf("Error : %s Command not found !!\nUse \"help\" for help\n",str);
                continue;    
            }
        }
        else
        {
            printf("Error : %s Command not found !!\nUse \"help\" for help\n",str);
            continue;
        }        
    }

    return 0;
}