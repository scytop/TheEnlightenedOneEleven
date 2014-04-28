#define GRAPH_SIZE 25

typedef struct graphNode{
command_t cmd;
pid_t pid;
struct graphNode **before;
int beforeSize;
}graphNode;

typedef struct string_q{
char* str;
struct string_q* next;
}string_q;

typedef struct listNode{
graphNode* gnode;
string_q* readlist; //don't forget to malloc these!
string_q* writelist;
struct listNode* next;
}listNode;

typedef struct dependencyGraph{
listnode** no_dependencies;
listnode** dependencies;
int ndSize;
int dSize;
}dependencyGraph;

bool hasSimilar(string_q* l1, string_q* l2)
{
//returns true if there exists an element that occurs
//in both l1 and l2
string_q* t1 = l1;
string_q* t2 = l2;

if(t1 == NULL)
	return false;
else if (t2 == NULL)
{
return hasSimilar(t1->next, l2);
}
if(strcmp(t1->str, t2->str) == 0)
	return true;
else
	return(t1, t2->next); 
}

bool hasSimilar(string_q* l1, string_q* l2)
{
string_q* start = l2;
while(l1 != NULL)
{
	while(l2 != NULL)
	{
	if(strcmp(l1->str, l2->str) == 0)
		return true;
	else
		l2 = l2->next;
	}
l2 = start;
l1 = l1->next;
}
return false;
}

void linsert(string_q* root, char* elem)
{
string_q* temp = root;
while(temp != NULL)
{
if(temp->next == NULL)
{
	string_q* newGuy = malloc(sizeof(string_q));
	newGuy->next = NULL;
	temp-> next = newGuy;
	strcpy(newGuy->str, elem);	
	break;
}
else
{
temp = temp->next;
}
}
}//not efficient rn, can come back later to "insert sort"

dependencyGraph buildItBro(listNode* start)
{
depencencyGraph* result = malloc(sizeof(dependencyGraph));
result-> ndSize = 0;
result-> dSize  = 0;
listNode* current = start;
listNode* checkNode = malloc(sizeof(listNode));

	if(current != NULL) checkNode = current->next;
	while(current->next != NULL)
	{
	while(checkNode->next !=NULL)
	{
	if(hasSimilar(current->writelist, checkNode->readlist)||
		hasSimilar(current->readlist, checkNode->writelist)||
		hasSimilar(current->writelist, checkNode->writelist))
	{
	checkNode->before[(checkNode->beforeSize)] = current;
	(checkNode->beforeSize)++;
	}
	checkNode = checkNode->next;
	}
	current = current->next;
	}
	//ok all these bitches should have their deps
	
	while(start != NULL)
	{
	if(start->beforeSize == 0)
	{
		result->no_dependencies[result -> ndSize] = start;
		(result->ndSize)++;
	}
	else
	{
		result->dependencies[result -> dSize] = start;
		(result->dSize)++;
	}
	start = start->next;
	}
return *result;
}


void proCom(command_t c, listNode* l){
if(c->type == SIMPLE_COMMAND){
	if(c->u.input != NULL) linsert(l->readlist, c->u.input);
	int i = 0;
	while(c->u.word[i][0] != '\0')//FIXME: THIS IS WHERE WE GET MESSED UP
	{
	linsert(l->readlist, c->u.word[i]);
	i++;
	}
	if(c->u.output != NULL) linsert(l->writelist, c->u.output);}
else if(c->type == SUBSHELL_COMMAND)
{
	if(c->u.input != NULL) linsert(l->readlist, c->u.input);
	if(c->u.input != NULL) linsert(l->writelist, c->u.output);
	proCom(c->u.subshell_command, l);
}
else
{
	proCom(c->u.command[0], l);
	proCom(c->u.command[1], l);
}
}

//	dependencyGraph* result = malloc(sizeof(dependencyGraph));
//	result->ndSize = 0;
//	result->dSize  = 0;
dependencyGraph create_graph(command_stream_t cs){
	listNode* current = malloc(sizeof(listNode));
	listNode* start = current;
	current->next = NULL;

	command_t command;
	//build the first linked list of all listNodes
	while(command = read_command_stream(cs))
	{
	listNode* l = malloc(sizeof(listNode));
	proCom(command->command, &(l->readlist), &(l->writelist));
	graphNode* g = malloc(sizeof(graphNode));
	g->cmd = command->command;
	g->beforeSize = 0;
	g->before = malloc(sizeof(graphNode)*DEFAULT_BUFFER_SIZE);
	l->gnode = g;
	//builds a single node	

	current-> next = l;
	l->next= NULL;
	current = l;
	//add it to the list
	}
	start = start->next;
	//start will be null if there are no thinggies
	//start has a linked list of read/write listnodes
	//build it forward, not backwards
	return buildItBro(start);
}

void execNd(dependencyGraph d)
{
int i = 0;
int size = d->ndSize;
for(i = 0; i < size; i++)
{
	pid_t pid = fork();
	if(pid == 0)
	{
	execute_command((d->no_dependencies)[i]->gnode->cmd, true);
	_exit((d->no_dependencies)[i]->gnode->cmd->status);
	}
	if(pid >0)
	{
	(d->no_dependencies)[i] ->gnode->pid = pid;
	}
}
}


void execD(dependencyGraph d)
{
int temp;
int i;
int size = d->dSize;
for(i = 0; i<size; i++)
{
	int j = 0;
	for(j = 0; d->dependencies[i]->gnode->before[j] != NULL; j++)
	{
	if(d->dependencies[i]->gnode->before[j]->pid == -1)
		j = -1;
	else
		continue;
	}
	for(j = 0; d->dependencies[i]->gnode->before[j] != NULL; j++)
	{
		waitpid(d->dependencies[i]->gnode->before[j]->pid, &temp, 0);
	}
	pid_t pid = fork();
	if(pid < 0)
		error(1, errno, "It Forked up");
	else if(pid == 0)
	{
	execute_command(d->dependencies[i]->gnode->cmd, true);
	_exit(d->dependencies[i]->node->cmd->status);
	}
	else
		d->dependencies[i]->gnode->pid = pid;
}
}

void execGraph(dependencyGraph d)
{
execNd(d);
execD(d);

int i=0;
int j = 0;
int temp;
for(i = 0; i < d.ndSize; i++)
{
	waitpid(g.noDependencies[i]->gnode->pid, &temp, 0);
}
for(j = 0; j < d.dSize; j++)
	waitpid(g.dependencies[j]->gnode->pid, &temp, 0);
}
