/*Shobhan Mandal
 *20172064 
 * 
 * */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>

//--------editing required header files 
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
//--------

/*-----Global Variables start------*/
struct termios initial,changedState;
struct winsize w;

int fileDescriptor, nodePointerCharacter1=0, nodePointerCharacter2=0,xPos=0,yPos=0,fileEditFlag=0;
char *paragraphText, bufferArray[400][400], *fname;

struct node{
	char *text;
	struct node *next,*prev;
}*head,*t,*pointer1,*pointer2;



/*-----Global Variables end------*/

void insertMode(int newOrOld);
int knowNodeLocationForInsertion();
void bufferNodeRearrangement();
void nodeTextRearrangement(int nodeLocationBuffer);

//get the window size in terms of rows and columns
void getWindowSize()
{
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
}

//The canonical mode needs to be reset since after the program ends it may cause your input not visible on the terminal
void resetCanonicalMode()
{
	    tcsetattr(STDIN_FILENO, TCSAFLUSH, &initial);
	    write(STDOUT_FILENO, "\x1b[2J", 4); 
		write(STDOUT_FILENO, "\x1b[H", 3); 
}

void setNonCanonicalMode()
{
	  tcgetattr(STDIN_FILENO, &initial); //get the attribute values regarding the STDIN_FILENO into the structure pointed by initial
	  atexit(resetCanonicalMode);
	  
	  changedState=initial;
	  changedState.c_lflag &= ~ECHO;	
	  changedState.c_lflag &= ~ICANON;
	  changedState.c_cc[VMIN] = 1;
      changedState.c_cc[VTIME] = 1;
      
	  tcsetattr(STDIN_FILENO, TCSAFLUSH, &changedState);	  
}

void displayLastLine( char *text)
{
	char cursorBuffer[11];
	snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",w.ws_row,0);
    write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
    write(STDOUT_FILENO, "                                                  ",50);
    snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",w.ws_row,0);
    write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
    write(STDOUT_FILENO, text,strlen(text));
    
}
//Displays the screen for the 1st time
void firstDisplay()
{
	int bufferI=0,bufferJ=0;
	nodePointerCharacter1=nodePointerCharacter2=0;
	pointer1=pointer2=head->next;
	
	for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				bufferArray[bufferI][bufferJ]='\0';
		    }
		}
		
		for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{

				  bufferArray[bufferI][bufferJ]=pointer2->text[nodePointerCharacter2];
				if(pointer2->text[nodePointerCharacter2]=='\n')
				{
				  pointer2=pointer2->next;
				  nodePointerCharacter2=0;
				  break;
			    }
			    nodePointerCharacter2++;
			}
			if(pointer2==NULL)
			  break;
		}
	
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);	
	
	for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				write(1,&bufferArray[bufferI][bufferJ],1);
			}
			
		}  
	
	write(STDOUT_FILENO, "\x1b[H", 3);
}



//Display in the screen when down key is pressed
void displayDown(int G)
{
	int bufferI=0,bufferJ=0;
	if(pointer2->next!=NULL)
	{
		//update the starting pointer of the buffer so that upward key can be used
		for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
		{
			if(pointer1->text[nodePointerCharacter1]=='\n')
			{
				pointer1=pointer1->next;
				nodePointerCharacter1=0;	
				break;
			}
			nodePointerCharacter1++;
		}	
		
		//From the 2nd line copy the elemetns into the previous line for the whole buffer
		for(bufferI=1;bufferI<w.ws_row-1;bufferI++)
		{			
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				bufferArray[bufferI-1][bufferJ]=bufferArray[bufferI][bufferJ];
			}
		}
		
		//Clearing out the last line of the Buffer
		for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
		{
			bufferArray[w.ws_row-2][bufferJ]='\0';
		}
		
			//Updating the last line of the buffer from the linked list
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				bufferArray[w.ws_row-2][bufferJ]=pointer2->text[nodePointerCharacter2];
				if(pointer2->text[nodePointerCharacter2]=='\n')
				{
				  pointer2=pointer2->next;
				  nodePointerCharacter2=0;
				  break;
			    }
			    nodePointerCharacter2++;
			}
		    
		     
		write(STDOUT_FILENO, "\x1b[2J", 4);
		write(STDOUT_FILENO, "\x1b[H", 3);
       if(G==0)
       {	   
		for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				write(1,&bufferArray[bufferI][bufferJ],1);
				//printf("%c",bufferArray[bufferI][bufferJ]);
			}
		}
	   }
	}
	
	if(G==1&&pointer2->next==NULL)
	{
		for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				write(1,&bufferArray[bufferI][bufferJ],1);
				//printf("%c",bufferArray[bufferI][bufferJ]);
			}
		}
	}
				
}

//Display in the screen when UP key is pressed
void displayUp()
{
	int bufferI=0,bufferJ=0,length=0,tempNodePointerCharacter=0;
	
	if(pointer1->prev->prev!=NULL||nodePointerCharacter1!=0)
	{
		//update the ending pointer of the buffer so that upward key can be used and later back can be used
		if(nodePointerCharacter2==0)
		{
			pointer2=pointer2->prev;
			length=strlen(pointer2->text);
			
			nodePointerCharacter2=length-length%w.ws_col;
	
		}
		else
		{
			nodePointerCharacter2-=w.ws_col;
		}
		
		//copy the row-3 into row-2 and do the same for the previous rows
		for(bufferI=w.ws_row-2;bufferI>0;bufferI--)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				bufferArray[bufferI][bufferJ]=bufferArray[bufferI-1][bufferJ];
			}
		}
		
		//char tempo1[1];
		for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
		{
			bufferArray[0][bufferJ]='\0';
		}
		
		if(nodePointerCharacter1==0)
		{
			pointer1=pointer1->prev;
			length=strlen(pointer1->text);
			nodePointerCharacter1=tempNodePointerCharacter=length-length%w.ws_col;
			length=length%w.ws_col;
			
		}
		else
		{
			  length=w.ws_col;
			  nodePointerCharacter1=tempNodePointerCharacter=nodePointerCharacter1-length;
		}
		
		for(bufferJ=0;bufferJ<(length);bufferJ++)
		{
			 bufferArray[0][bufferJ]=pointer1->text[tempNodePointerCharacter++];
			 //nodePointerCharacter1--;   
		}
		
		write(STDOUT_FILENO, "\x1b[2J", 4);
		write(STDOUT_FILENO, "\x1b[H", 3);
       	   
		for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				write(1,&bufferArray[bufferI][bufferJ],1);
				//printf("%c",bufferArray[bufferI][bufferJ]);
			}
		}	
	}
}

//Saving the file to the disk
void fileSave()
{
	fileDescriptor=open(fname,O_RDWR|O_TRUNC);
	if(fileDescriptor==-1)
	  displayLastLine("Issue in File Saving");
	else
	{  	
	 struct node *temp=head->next;
	 while(temp!=NULL)
	 {
		write(fileDescriptor,temp->text,strlen(temp->text));
		temp=temp->next;
	 }
    }
    displayLastLine("File Saved----");
}

//Takes the input for the command mode commands
int commandMode()
{
	char readingInput[1],readCommand[15];
	int i=0,commandModeReturnValue=0;;
	
	displayLastLine(":");
	
	while(1)
	{
		if(read(0, &readingInput, 1) >= 1)
		{
			if(readingInput[0]==27)
				    break;
			else if(readingInput[0]==10)
			{
				if(readCommand[0]=='w')
				{
					fileEditFlag=0;
					fileSave();
					break;
				}
				else if(readCommand[0]=='q' && readCommand[1]=='!')        
				  {
				    commandModeReturnValue=1; 
				    break;
				  } 
				else if(readCommand[0]=='q')
				{
				  if(fileEditFlag==1)
				  {
				    displayLastLine("File has not been saved since last changes!!!");  	
				    break;
				  } 
				  else
				   {
					commandModeReturnValue=1; 
					break;
				   }
				}   
				else if(readCommand[0]=='!')
				{
				  for(int i=0;i<strlen(readCommand);i++)
				   readCommand[i]=readCommand[i+1];
				  write(1,"\x1b[?1049h",8);
				  write(STDOUT_FILENO, "\x1b[H", 3);
				  write(1,"Command Executed:",17);
				  write(1,readCommand,strlen(readCommand));
				  write(STDOUT_FILENO, "\x1b[3;1H",6);
				  system(readCommand);
				  sleep(5);
				  //if(read(0,&readingInput,1)==10)
				    write(1,"\x1b[?1049l",8);
				    
				  i=0;     
			    } 
			}        	    
			else
			{
				write(STDOUT_FILENO, readingInput,1);
				readCommand[i++]=readingInput[0];
			}	
	    }
	}
	return commandModeReturnValue;
}

//This is the general Keyboard reader which is present in the normal mode
void keyBoardReader()
{	
	char readingInput[3],rKeyPress[1],tempKeyPress,cursorBuffer[11];
	xPos=yPos=1;
	
	int commandModeReturnValue=0;

	displayLastLine("---Normal mode---");
	write(STDOUT_FILENO, "\x1b[H", 3);
	
	while(1)
	{	
		if(read(0, &readingInput, 3) >= 1)
		{
			if (readingInput[0]=='q')
			break;


			else
			{
			if(readingInput[0]==27 && readingInput[1]==91)
			{
				switch(readingInput[2])
				{
					case 'A':readingInput[0]='k';break;
					case 'B':readingInput[0]='j';break;
					case 'C':readingInput[0]='l';break;
					case 'D':readingInput[0]='h';break;
				}  
			}

			switch(readingInput[0])
			{
				case 'h'://left /x1b[columns,rowsf
				   if(yPos==1&&xPos==1)
				     displayUp();
				   else if(yPos==1)
				   {
				     yPos=w.ws_col;
				     --xPos;
				   }
				   else
				    --yPos;
				   snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,yPos);
				   //snprintf(cursorBuffer, 11, "\x1b[%d;%df",(xPos>1)?--xPos:xPos,yPos);
				   displayLastLine("---Normal mode---");
				   write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));	
				break;

				case 'l'://right
					if(bufferArray[xPos-1][yPos]=='\n'||yPos==w.ws_col)
					{
						yPos=0;
						if(xPos==w.ws_row-1)
						  displayDown(0);
						xPos=(xPos!=w.ws_row-1)?++xPos:xPos;
					}
					else
					
						++yPos;
					snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,yPos);
					//snprintf(cursorBuffer, 11, "\x1b[%d;%df",xPos++,yPos);
					displayLastLine("---Normal mode---");
					write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));	
				break;

				case 'k'://up
					if(xPos==1)
					  displayUp();
					snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",(xPos>1)?--xPos:xPos,yPos);
					//snprintf(cursorBuffer, 11, "\x1b[%d;%df",xPos,(yPos>1)?--yPos:yPos);
					displayLastLine("---Normal mode---");
					write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
				break;

				case 'j'://down
					if(xPos==w.ws_row-1)
					  displayDown(0);
					snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",(xPos!=w.ws_row-1)?++xPos:xPos,yPos);
					//snprintf(cursorBuffer, 11, "\x1b[%d;%df",xPos,yPos++);
					displayLastLine("---Normal mode---");
					write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
				break;

				case 'g'://1st line
					 if(tempKeyPress=='g')
					  {
					   firstDisplay();
					   xPos=yPos=1;
					   snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,yPos);
					  }
					  displayLastLine("---Normal mode---");
					  write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
				break;
					  
				case 'G':
					 while(pointer2->next!=NULL)
					   displayDown(1);
					 displayDown(1);
					 displayLastLine("---Normal mode---");  
					 snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",w.ws_row-1,1);
					 write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
					 
				break;

				case 'r':
				     displayLastLine("---Replace mode---");
				     write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
					 //rKeyPress[0]=getchar();
					 while(1)
					 {
					  if(read(1,&rKeyPress,3)>=1)
					  {
						if(rKeyPress[1]!='['&&rKeyPress[0]!='\x1b')   
					     break;
					    rKeyPress[1]='A';   
					  }
				     }
				     fileEditFlag=1;
					 bufferArray[0][0]=rKeyPress[0];
					 write(STDOUT_FILENO, rKeyPress,1);
					 int temp=knowNodeLocationForInsertion();
					 bufferNodeRearrangement(0,temp);
					 //nodeTextRearrangement(temp);
					 
				break;

				case 'i':
					insertMode(1);
				break;

				case ':':
					commandModeReturnValue=commandMode();
					snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,xPos);
					write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
				break;        
				}
			tempKeyPress=readingInput[0];
			}
			if(commandModeReturnValue==1)
			{
			 displayLastLine("--------------------!!!!Adieos!!!!----------------------------");
			 sleep(2);	
			 break;
		    }
		    /*else
		     displayLastLine("---Normal mode---");*/
	    }			
	}	 
}

//After the changes made in the file this function displays it in the front end
void displayInsertMode()
{
	int bufferI=0,bufferJ=0;
	pointer2=pointer1;
	nodePointerCharacter2=nodePointerCharacter1;
	
	for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				bufferArray[bufferI][bufferJ]='\0';
		    }
		}
		
		for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{

				  bufferArray[bufferI][bufferJ]=pointer2->text[nodePointerCharacter2];
				if(pointer2->text[nodePointerCharacter2]=='\n')
				{
				  pointer2=pointer2->next;
				  nodePointerCharacter2=0;
				  break;
			    }
			    nodePointerCharacter2++;
			}
			if(pointer2==NULL)
			  break;
		}
	
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);	
	
	//write(STDOUT_FILENO, "\r", 1);
	for(bufferI=0;bufferI<w.ws_row-1;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				write(1,&bufferArray[bufferI][bufferJ],1);
				//printf("%c",bufferArray[bufferI][bufferJ]);
			}
			
		}
}

//Depending on the key pressed various insert mode types are executed, it results in modification of the data in the 
//datastructure
void bufferNodeRearrangement(int mode, int nodeLocationBuffer)
{
	char inputCharacter=bufferArray[0][0];
	int i=0,j=0;
	
	  if(mode==0) //Replace
	  {
		  t->text[nodeLocationBuffer-1]=inputCharacter;
		  displayInsertMode();
	  }
      else if(mode==1)//Insertion
      {	
		fileEditFlag=1;
		
		if(inputCharacter!=10)
		{
			t->text = (char *)realloc(t->text, strlen(t->text)+2);
			
			for(i=strlen(t->text)+1;i>nodeLocationBuffer-1;i--)
				t->text[i]=t->text[i-1];
			
			t->text[nodeLocationBuffer-1]=inputCharacter;
			
			if(xPos==w.ws_row-1 && yPos==w.ws_col)
			  displayDown(0);
			else
			  displayInsertMode();
			
	    }
	    else
	    {   
			//displayLastLine("Enter");sleep(1);
			char beginNode[nodeLocationBuffer+1];
			
			struct node *temp=(struct node *)malloc(1*sizeof(struct node));;
			temp->next=NULL;
			temp->text = (char *) malloc(strlen(t->text)-nodeLocationBuffer);
			
			
			for(i=nodeLocationBuffer-1,j=0;i<strlen(t->text);i++,j++)
			  temp->text[j]=t->text[i];
			
			for(i=0;i<nodeLocationBuffer-1;i++)
			  beginNode[i]=  t->text[i];
			
			beginNode[i]='\n';
		
			strcpy(t->text,beginNode);
			
			if(t->next!=NULL)
			 t->next->prev=temp;
			temp->next = t->next;
			t->next=temp;
			temp->prev = t;
			
			t=t->next;
			
			if(xPos==w.ws_row-1)
			{
			  displayInsertMode();	
			  displayDown(0);
		    }
			else
			  displayInsertMode();
			
			//displayInsertMode();
		}	  
	  }
	  else if(mode==2)//backspace
	  {
		 //printf("NodeLocationBuffer:%d",nodeLocationBuffer);
		 //sleep(2);
		 if(nodeLocationBuffer-1==0)
		 {
			 //int l1=strlen(t->text);
			 if(t->prev!=head)
			 {
			    t->prev->text = (char *)realloc(t->prev->text, strlen(t->prev->text)+strlen(t->text)+2);
			 
			    for(i=strlen(t->prev->text)-1;i>=0;i--)
			    {
				   if(t->prev->text[i]=='\n')
				   	   t->prev->text[i]='\0';
				}
			    
			    for(i=strlen(t->prev->text)-1,j=0;i<strlen(t->prev->text)+strlen(t->text)+2;i++,j++)
			    {
				    t->prev->text[i]=t->text[j];
				}
			    
			    t->prev->next=t->next;
			    t->next->prev=t->prev;
			    
			    t=t->prev;
			    //t->text[strlen(t->text)-1]
			}
		 }
		 else
		 {
			 int length=strlen(t->text);
			 for(i=nodeLocationBuffer-2;i<length;i++)
			  t->text[i]=t->text[i+1];
			 t->text[i]='\0'; 
		 }
		 
		 
	     if(xPos==1&&yPos==1)
			{
			  displayInsertMode();	
			  displayUp();
		    }
		 else //if(xPos==w.ws_row-1)  
	       displayInsertMode();  
	  }
	  else if(mode==3)//delete
	  {
		  displayLastLine("Delete Detected");sleep(5);
	  }
	  else if(mode==4)//Creation of the new file
	  {
		  
		 fileEditFlag=1;
		 if(head->next==NULL)
		 {
			  t = (struct node *)malloc(1*sizeof(struct node));
			  t->next=NULL;
			  t->text = (char *) malloc(10);
			  for(i=0;i<10;i++)
			   t->text[i] = '\0';
			  head->next=t;
			  t->prev=head;
			  nodePointerCharacter1=0;
			  pointer1=head->next;
		 }
		 
		   if(inputCharacter!='\n')
		   {
			   t->text = (char *)realloc(t->text, strlen(t->text)+2);
			   
			   t->text[nodeLocationBuffer]=inputCharacter;
		   }
		   else
		   {
			   t->text[nodeLocationBuffer]=inputCharacter;
			   nodeLocationBuffer=0;  
			   pointer2=t;
			   
			   t = (struct node *)malloc(1*sizeof(struct node));
		       t->next=NULL;
		       
		       t->text = (char *)malloc(10);
		       for(i=0;i<10;i++)
				t->text[i] = '\0';
		       pointer2->next=t;
		       t->prev=pointer2;		         
		   }
	     
		 if(xPos==w.ws_row-1)
			{
			  displayInsertMode();	
			  displayDown(0);
		    }
			else
			  displayInsertMode();
	  }
    
}

//At any moment depending on the position of the cursor on the screen it can specify the node and the 
//offset of the character in the node
int knowNodeLocationForInsertion()
{
	t=pointer1;
	int loc=nodePointerCharacter1,bufferI=0,bufferJ=0;
	//char cursorBuffer[55];
	for(bufferI=0;bufferI<xPos;bufferI++)
		{
			for(bufferJ=0;bufferJ<w.ws_col;bufferJ++)
			{
				if(bufferJ==yPos&&bufferI==xPos-1)
			      break;
				if(t->text[loc]=='\n')
				{
			      t=t->next;
				  loc=0;
				  break;
			    }
			    
			    loc++;
			}
			if(t==NULL)
			  break;
		}

	return loc;
}

//The insert mode takes various inputs like alphanumeric characters, enter, backspace which then it passes on to
//bufferNodeRearrangement to be put into the DataStructure
void insertMode(int newOrOld)
{
	char readingInput[3],cursorBuffer[11];
	int nodeLocationBuffer=0;
	if(newOrOld==0)
	{
		xPos=yPos=1;nodePointerCharacter1=0;
		displayLastLine("-----Insert New File-----");
		write(STDOUT_FILENO, "\x1b[H", 3);
		
		
		while(1)
		{
			//printf("Node Location Buffer:%d\n",nodeLocationBuffer);
			//getWindowSize();
			if(read(0, &readingInput, 3) >= 1 )
		    {
				if(readingInput[0]==27 && readingInput[1]!='[')
					break;
				else if((readingInput[0]>=65&&readingInput[0]<=90)||(readingInput[0]>=97&&readingInput[0]<=122)||(readingInput[0]>=48&&readingInput[0]<=57)||readingInput[0]==32) 
				{
					bufferArray[0][0]=readingInput[0];
					//write(STDOUT_FILENO, readingInput,1);
					bufferNodeRearrangement(4,nodeLocationBuffer);
					
					++yPos;
					if(yPos==w.ws_col&&xPos==w.ws_row-1)
					 {yPos=1;}
					else if (yPos==w.ws_col&&xPos<w.ws_row-1) 
					 { yPos=1;++xPos;} 
					 nodeLocationBuffer++;
				}
				else if(readingInput[0]==10)
				{
				   bufferArray[0][0]=readingInput[0];
				   //nodeLocationBuffer=knowNodeLocationForInsertion();
				   bufferNodeRearrangement(4,nodeLocationBuffer);
				   nodeLocationBuffer=0;
				   
				   if(xPos<w.ws_row-1)
				      ++xPos;
				   yPos=1;
			    }
			    else if(readingInput[0]==127)
			    {
					bufferArray[0][0]=readingInput[0];
					nodeLocationBuffer=knowNodeLocationForInsertion();
					if(yPos==1&&xPos>1)
					{
						--xPos;
						if(nodeLocationBuffer-1==0&&t->prev!=head)
						{
							yPos=strlen(t->prev->text)%w.ws_col;
						}
						else
						yPos=w.ws_col;    
					}
					else if(yPos>1)
					{
						--yPos;
					}
					if(!(t->prev==head&&nodeLocationBuffer==0)) 
					  bufferNodeRearrangement(2,nodeLocationBuffer);
				}
			}
			
			snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,yPos);
	        write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
		}
		keyBoardReader();
	}
	else
	{
		displayLastLine("-----Insert-----");
		snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,yPos);
		write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
		
		nodeLocationBuffer=knowNodeLocationForInsertion();
		
		while(1)
		{
			if(read(0, &readingInput, 3) >= 1)
		    {
				if(readingInput[0]==27 && readingInput[1]!='[')
				{
					    break;
				}
				else if((readingInput[0]>=65&&readingInput[0]<=90)||(readingInput[0]>=97&&readingInput[0]<=122)||(readingInput[0]>=48&&readingInput[0]<=57)||readingInput[0]==32) 
				{
					bufferArray[0][0]=readingInput[0];
					bufferNodeRearrangement(1,nodeLocationBuffer++);
					
					snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,yPos);
					write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
					
					
					yPos++;
					if(yPos==w.ws_col&&xPos==w.ws_row-1)
					 { yPos=1;}
					else if (yPos==w.ws_col&&xPos<w.ws_row-1) 
					 { yPos=1;++xPos;}
				}
				else if(readingInput[0]==10)
				{
				   bufferArray[0][0]=readingInput[0];
				   nodeLocationBuffer=knowNodeLocationForInsertion();
				   bufferNodeRearrangement(1,nodeLocationBuffer++);
				   nodeLocationBuffer=1;	
				   //Struct node *temp=(struct node *)malloc(1*sizeof(struct node));;
				   if(xPos<w.ws_row-1)
				    ++xPos;
						//yPos=1;}
					
					yPos=1;	
			    }
			    else if(readingInput[0]==127)
			    {
					bufferArray[0][0]=readingInput[0];
					nodeLocationBuffer=knowNodeLocationForInsertion();
					if(yPos==1&&xPos>1)
					{
						--xPos;
						if(nodeLocationBuffer-1==0&&t->prev!=head)
						{
							yPos=strlen(t->prev->text)%w.ws_col;
						}
						else
						yPos=w.ws_col;    
					}
					else if(yPos>1)
					{
						--yPos;
					}
					if(!(t->prev==head&&nodeLocationBuffer==0)) 
					  bufferNodeRearrangement(2,nodeLocationBuffer);
				}
			    snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",xPos,yPos);
	        	write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
			    
			}
			readingInput[1]=90;
			
		 }
		snprintf(cursorBuffer, sizeof(cursorBuffer), "\x1b[%d;%dH",w.ws_row,0);
		write(STDOUT_FILENO, cursorBuffer,strlen(cursorBuffer));
		displayLastLine("-----Normal Mode------");
		
	}
}



//Normal Mode:-Fills the data read from the file in the datastructure
void normalMode()
{
	char readingInput[1]; //readingInput[128]
	int numberOfBytes=0,initialSize=10,i=0;
	
	if(access(fname,F_OK)!=-1)
		fileDescriptor=open(fname,O_RDONLY);
	else
		fileDescriptor=open(fname,O_RDWR|O_CREAT);	
	
	if(fileDescriptor==-1)
	 printf("\n *****Error in Opening the file");
	else
	{
		/*while((numberOfBytes=read(fileDescriptor,readingInput,128))>0)
		  write(1,readingInput,numberOfBytes);*/
		 head = (struct node *)malloc(1*sizeof(struct node));;
		 head->prev=NULL;
		 head->next=NULL;
		 
		 pointer1= (struct node *)malloc(1*sizeof(struct node));
		 t = (struct node *)malloc(1*sizeof(struct node));
		 t->next=NULL;
		 t->text = (char *) malloc(initialSize);
		 t->text[0] = ' ';
		  
		 while((numberOfBytes=read(fileDescriptor,readingInput,1))>0)
		 {
		   if(readingInput[0]!='\n')//&&(!iscntrl(readingInput[0])))
		   {
			   if(i==initialSize)
			   {
				   initialSize+=initialSize;
			       t->text = (char *)realloc(t->text, initialSize+1);
			   }
			   t->text[i++]=readingInput[0];
		   }
		   else
		   {
			   t->text[i++]=readingInput[0];  
			   //t->text=pStr;
			   if(head->next==NULL)
			   {
				 head->next=t;
				 t->prev=head;
				 pointer1=t;
			   }
			   else
			   {
				 pointer1->next=t;
				 t->prev=pointer1;
				 pointer1=t;
			   }
			   t = (struct node *)malloc(1*sizeof(struct node));
		       t->next=NULL;
		       i=0;
		       initialSize=10;
		       t->text = (char *)malloc(initialSize);
		       t->text[0]=' ';		         
		   }
	     }
	     pointer1->next=t;
		 t->prev=pointer1;
	}
	
	if(head->next==NULL)
	  insertMode(0);
	else
	{
	  firstDisplay();
	  keyBoardReader();
	}	
}

int testingFunction(char *argv[])
{
	if (argv[1] == NULL)
		printf("Argv is NULL");
}

int main(int argc, char *argv[]) //This helps in capturing the argument in this case the file name
{
	if(argv[1]==NULL)
	 printf("No File inputted\n");
	else
	{
     printf("Name of the File : %s \n",argv[1]);
      
	 setNonCanonicalMode();
	
	 write(STDOUT_FILENO, "\x1b[2J", 4); //x1b-> Escape Character, [ -> its the format, 2J-  CLears the complete screen
	 write(STDOUT_FILENO, "\x1b[H", 3); //H helps in reposition of the cursor
	
	
	 getWindowSize();	
	
	 //printf(" Rows: %d, Columns: %d\n\n",w.ws_row,w.ws_col);
	 fname=argv[1];
	 normalMode();
    }	
	return 0;	
}
