#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fuse.h>
#include "assert.h"

long availableMemory;
int maximumNodes;
int sizeOfNode = 0;
char *newNode;
int pathLength = 0;

time_t currentTime;

struct ramdisk_node
{
	char *name;	
	char *type;
	struct stat *stbuf;
	char *data;
    struct ramdisk_node *parent;
	struct ramdisk_node *child;
	struct ramdisk_node *sibling;
};
struct ramdisk_node *root;

struct ramdisk_node* createNode()
{
    struct ramdisk_node *newNode;
    newNode = (struct ramdisk_node *) malloc(sizeof(struct ramdisk_node));
    newNode->stbuf = (struct stat *)malloc(sizeof(struct stat));

    time(&currentTime);

    newNode->stbuf->st_atime = currentTime;
	newNode->stbuf->st_ctime = currentTime;
	newNode->stbuf->st_mtime = currentTime;    

    newNode->child = NULL;
    newNode ->sibling = NULL;

    return newNode;
}

void ramdisk_init()
{

    root = createNode(); 
    root->parent = NULL;
    root->name = malloc(sizeof(char)*strlen("/"));
    root->type = "dir";
    root->stbuf->st_mode = S_IFDIR | 0755;
    root->stbuf->st_nlink = 2;
    root->stbuf->st_size = sizeOfNode;
}

struct pathStructure
{
    char * nodeValue;
    struct pathStructure *next;

};
struct pathStructure* head = NULL;

void createPathStructure(char *pathValue)
{
    struct pathStructure *node;
    node = (struct pathStructure*)malloc(sizeof(struct pathStructure));
    node->nodeValue = pathValue;
    node->next = NULL;

    if(head==NULL)
    {
        head = node;
    }
    else
    {
        struct pathStructure *temp;
        temp = head;
        while(temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = node;
    }   
}

void emptyPathStructure()
{
    struct pathStructure *temp;
    struct pathStructure *nextNode;
    
    if (head != NULL)
    {
        temp = head;        
        while(temp->next!=NULL)
        {
            nextNode = temp ->next;
            free(temp);
            temp = nextNode;
        }
        if(temp != head)
        {
            free(temp);
        }
        head = NULL;
        pathLength=0;
    }
}

int processPathString(char *path)
{
    emptyPathStructure();
    char *pathToken;

    pathToken = strtok(path, "/");

    if(pathToken == NULL && strcmp(path, "/"))
    {
        return 0;
    }
    else
    {
        while( pathToken != NULL )
        {
            createPathStructure(pathToken);
            pathToken = strtok(NULL, "/");
            pathLength++;
        }
        return 1;
    }
}

//Validates path 
int validatePath(const char *inputPathValue)
{
    char path[512];
    strcpy(path, inputPathValue);
    int pathStatus = processPathString(path);   

    if(pathStatus == 0)
    {
        return pathStatus;
    }
    else
    {
        int flag = 0;
        struct ramdisk_node *currentNode = root;
        struct ramdisk_node *currentChildNode;

        struct pathStructure *nextNode;
        nextNode = head;

        int i=0;
    
        for(i=0;i<pathLength;i++)
        {
            currentChildNode = currentNode->child;

            while (currentChildNode != NULL)
            {
                if(strcmp(currentChildNode->name,nextNode->nodeValue)==0)
                {
                    flag = 1;
                    break;
                }
                else
                {
                    currentChildNode = currentChildNode->sibling;
                }
            }
            if(flag == 0)
            {
                return 1;
            }
            currentNode = currentChildNode;
            flag = 0;
            nextNode= nextNode->next;
        }
        return 0;
    }
    return -1;
}

// Gets the node
struct ramdisk_node* getNode(const char *inputPathValue)
{
    char path[512];
    strcpy(path, inputPathValue);
    int pathStatus = processPathString(path);   

    if(pathStatus == 0)
    {
        return root;
    }
    else
    {
        int flag = 0;
        struct ramdisk_node *currentNode = root;
        struct ramdisk_node *currentChildNode;

        struct pathStructure *nextNode;
        nextNode = head;

     
      int i=0;
    
        for(i=0;i<pathLength;i++)
        {
            currentChildNode = currentNode->child;

            while (currentChildNode != NULL)
            {
                if(strcmp(currentChildNode->name,nextNode->nodeValue)==0)
                {
                    flag = 1;
                    break;
                }
                else
                {
                    currentChildNode = currentChildNode->sibling;
                }
            }
            if(flag == 0)
            {
                return NULL;
            }
            currentNode = currentChildNode;
            flag = 0;
            nextNode= nextNode->next;
        }
        return currentNode;
    }
    return NULL;
}

// Gets the parent node
struct ramdisk_node* getParentNode(const char *inputPathValue)
{
    char path[512];
    strcpy(path, inputPathValue);
    int pathStatus = processPathString(path);   

    if(pathStatus == 0)
    {
        return root;
    }
    else
    {
        int flag = 0;
        struct ramdisk_node *currentNode = root;
        struct ramdisk_node *currentChildNode;
        struct pathStructure *nextNode;
        nextNode = head;

     
     int i=0;
    
        for(i=0;i<pathLength;i++)
        {
            currentChildNode = currentNode->child;

            while (currentChildNode != NULL)
            {
                if(strcmp(currentChildNode->name,nextNode->nodeValue)==0)
                {
                    flag = 1;
                    break;
                }
                else
                {
                    currentChildNode = currentChildNode->sibling;
                }
            }
            if(flag == 0)
            {
                newNode = malloc(sizeof(char)*strlen(nextNode->nodeValue));
                strcpy(newNode,nextNode->nodeValue);
                nextNode = nextNode->next;
                if(nextNode == NULL)
                {
                    return currentNode;
                }
                else
                {
                    return NULL;
                }
               
            }
            currentNode = currentChildNode;
            flag = 0;
            nextNode= nextNode->next;
        }
        return currentNode;
    }
    return NULL;
}



static int ramdisk_getattr(const char *path, struct stat *stbuf)
{
    if(validatePath(path) == 0)
    {
        struct ramdisk_node* currentNode = getNode(path);
        stbuf->st_mode = currentNode->stbuf->st_mode;
        stbuf->st_nlink = currentNode->stbuf->st_nlink;
        stbuf->st_size = currentNode->stbuf->st_size;
        stbuf->st_atime = currentNode->stbuf->st_atime;
        stbuf->st_mtime = currentNode->stbuf->st_mtime;
        stbuf->st_ctime = currentNode->stbuf->st_ctime;
        return 0;
    }
    return -ENOENT;
}

void add_node(struct ramdisk_node *inDir, struct ramdisk_node *new_node)
{
    new_node->parent = inDir;
    struct ramdisk_node *temp;
    temp = inDir->child;
    if(inDir->child == NULL)
    {
        inDir->child = new_node;
    }
    else
    {
        while(temp->sibling != NULL)
        {
            temp = temp->sibling;
        }
        temp->sibling = new_node;
    }
    availableMemory = availableMemory - sizeOfNode;

    time(&currentTime);

    inDir->stbuf->st_ctime = currentTime;
	inDir->stbuf->st_mtime = currentTime;

}

//Removing a node from File System
void remove_node(struct ramdisk_node *nodeToDelete)
{
     availableMemory = availableMemory + sizeOfNode;

    if(strcmp(nodeToDelete -> type,"file")==0)
    {
        availableMemory = availableMemory + (sizeof(char) * (strlen(nodeToDelete->data)));
    }

    struct ramdisk_node *startngNode, *tempNode;

    time(&currentTime);

    nodeToDelete->parent->stbuf->st_ctime = currentTime;
    nodeToDelete->parent->stbuf->st_mtime = currentTime;

    startngNode = nodeToDelete->parent;
    if( startngNode->child == nodeToDelete)
    {
        if(nodeToDelete->sibling == NULL)
        {
            startngNode->child = NULL;
            startngNode->stbuf->st_nlink --;
            free(nodeToDelete->stbuf);
            free(nodeToDelete);
            return;
        }
        else
        {
            startngNode->child = nodeToDelete->sibling;
            startngNode->stbuf->st_nlink--;
            free(nodeToDelete->stbuf);
            free(nodeToDelete);
            return;
        }       
    }
    else
    {
        tempNode = startngNode->child;
        while(tempNode->sibling != nodeToDelete)
        {
            tempNode = tempNode->sibling;
        }
        tempNode->sibling = nodeToDelete->sibling;
        startngNode->stbuf->st_nlink--;
        free(nodeToDelete->stbuf);
        free(nodeToDelete);
        return;
    }
}    

int ramdisk_unlink(const char* path)
{
    if(validatePath(path)!= 0)
    {
        return -ENOENT;
    }
    remove_node(getNode(path));
    return 0;
}

int ramdisk_rmdir(const char *path)
{
    if(validatePath(path)!= 0)
    {
        return -ENOENT;
    }
    struct ramdisk_node *nodeToDelete;
    nodeToDelete = getNode(path);
    if(nodeToDelete->child != NULL)
    {
        return -ENOTEMPTY;
    }
    remove_node(nodeToDelete);
    return 0;
}


int ramdisk_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    if((availableMemory - sizeof(sizeOfNode)) < 0)
    {
        return -ENOMEM;
    }
    if(validatePath(path) ==0)
    {
        return 0;
    }
    struct ramdisk_node * nodeToAdd;
    nodeToAdd = createNode();
    nodeToAdd->parent = getParentNode(path);
    if(nodeToAdd->parent==NULL)
    {
        return 0;
    }
    nodeToAdd->name = malloc(sizeof(char)*strlen(newNode));
    strcpy(nodeToAdd->name,newNode);
    nodeToAdd->type = "file";
    nodeToAdd->stbuf->st_mode = S_IFREG | mode;
    nodeToAdd->stbuf->st_nlink = 1;
    nodeToAdd->stbuf->st_size=0;
    nodeToAdd->data = malloc(0);
    add_node(nodeToAdd->parent,nodeToAdd);
    return 0;
}

int ramdisk_mkdir(const char* path, mode_t mode)
{
    if((availableMemory - sizeof(sizeOfNode)) < 0)
    {
        return -ENOMEM;
    }
    if(validatePath(path) ==0)
    {
        return 0;
    }
    struct ramdisk_node * nodeToAdd;
    nodeToAdd = createNode();
    nodeToAdd->parent = getParentNode(path);
    if(nodeToAdd->parent==NULL)
    {
        return 0;
    }
    nodeToAdd->name = malloc(sizeof(char)*strlen(newNode));
    strcpy(nodeToAdd->name,newNode);
    nodeToAdd->type = "file";
    nodeToAdd->stbuf->st_mode = S_IFDIR | 0755;
    nodeToAdd->stbuf->st_nlink = 2;
    nodeToAdd->stbuf->st_size=sizeOfNode;
    nodeToAdd->data = malloc(0);
    add_node(nodeToAdd->parent,nodeToAdd);
    return 0;
}



int ramdisk_open(const char* path, struct fuse_file_info* fi)
{
	if(validatePath(path) == 0)
	{
		return 0;
	}
	return -ENOENT;
}

int ramdisk_opendir(const char* path, struct fuse_file_info* fi)
{
	return ramdisk_open(path,fi);
}



static int ramdisk_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    if(validatePath(path) != 0)
	{
		return -ENOENT;
	}
    int flag = 0;
    
    struct  ramdisk_node *nodeToWrite;
    nodeToWrite = getNode(path);
    if(strcmp(nodeToWrite->type,"dir")==0)
    {
        return -EISDIR;;
    }

    size_t nodeToWriteCurrentSize = nodeToWrite->stbuf->st_size;
    if(nodeToWriteCurrentSize == 0)
    {
        if(availableMemory < (sizeof(char)*size))
        {
            return -ENOMEM;
        }
        nodeToWrite->data = malloc(sizeof(char)*size);
        memcpy(nodeToWrite->data,buf,size);
        nodeToWrite->stbuf->st_size = size;
        availableMemory = availableMemory - (sizeof(char)*size);
        flag = 1;
    }
    else if(nodeToWriteCurrentSize > (offset+size))
    {
        memcpy(nodeToWrite->data + offset, buf, size);
        flag = 1;
    }
    else
    {
        if(availableMemory < (sizeof(char)*(offset + size - nodeToWriteCurrentSize)))
        {
            return -ENOMEM;
        }
        nodeToWrite->data = realloc(nodeToWrite->data,sizeof(char) * (offset + size));
        memcpy(nodeToWrite->data + offset, buf, size);
        availableMemory = availableMemory - (sizeof(char)*(offset + size - nodeToWriteCurrentSize));
        flag = 1;
    }
    if(flag == 1)
    {
        time(&currentTime);
        nodeToWrite->stbuf->st_atime = currentTime;
        nodeToWrite->stbuf->st_ctime = currentTime;
        nodeToWrite->stbuf->st_mtime = currentTime;
        nodeToWrite->parent->stbuf->st_ctime = currentTime;
        nodeToWrite->parent->stbuf->st_mtime = currentTime;
    }  
    return 0;  
}

static int ramdisk_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    if(validatePath(path) != 0)
	{
		return -ENOENT;
	}
    struct  ramdisk_node *nodeToRead;
    nodeToRead = getNode(path);
    if(strcmp(nodeToRead->type,"dir")==0)
    {
        return -EISDIR;;
    }
    size_t nodeToReadCurrentSize = nodeToRead->stbuf->st_size;
    if(offset < nodeToReadCurrentSize )
    {
        if((offset+size) > nodeToReadCurrentSize)
        {
            size = nodeToReadCurrentSize - offset;
        }
        memcpy(buf,nodeToRead->data,size);
        time(&currentTime);
        nodeToRead->stbuf->st_atime = currentTime;
    }
    else
    {
        return 0;
    }
    return 0;
}

int ramdisk_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
{

	if(validatePath(path) != 0)
	{
		return -ENOENT;
	}
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	struct ramdisk_node *parent_node, *temp_node;
	parent_node = getNode(path);
	temp_node = parent_node->child;
	if(temp_node == NULL)
	{
		return 0;
	}
	filler(buf, temp_node->name, NULL, 0);
	while(temp_node->sibling != NULL)
	{
		temp_node = temp_node->sibling;
		filler(buf, temp_node->name, NULL, 0);
	}
	return 0;
}


int ramdisk_truncate(const char* path, off_t size)
{
	if(validatePath(path) != 0)
	{
		return -ENOENT;
	}
	struct ramdisk_node *file = getNode(path);
	if(strcmp(file->type,"dir")==0)
	{
		return -EISDIR;
	}
	size_t curr_size = file->stbuf->st_size;
	if(curr_size > size)
	{
		file->data = realloc(file->data, (size));
		file->stbuf->st_size = size;
		availableMemory = availableMemory + curr_size - size;
	}
	return 0;
}


// Referred from the hello.c 
static struct fuse_operations ramdisk_oper = {
	.getattr	= ramdisk_getattr,
	.readdir	= ramdisk_readdir,
	.opendir	= ramdisk_opendir,
	.mkdir		= ramdisk_mkdir,
	.rmdir		= ramdisk_rmdir,
	.create		= ramdisk_create,
	.open		= ramdisk_open,
    .read		= ramdisk_read,
	.write		= ramdisk_write,
 	.unlink		= ramdisk_unlink, 
	.truncate	= ramdisk_truncate,
	
};

int main(int argc, char *argv[])
{
    char *custom_argv[2];

    if(argc!=3)
    {
        exit(1);
    }

    custom_argv[0] = argv[0];
    custom_argv[1] = argv[1];

    int size = atoi(argv[2]);

    if(size < 0)
    {
         exit(1);
    }

    availableMemory = size*1024*1024;

    sizeOfNode = sizeof(struct ramdisk_node) + sizeof( struct stat);


    ramdisk_init();


  
    fuse_main(argc-1, custom_argv, &ramdisk_oper, NULL);
	return 0;

    
}