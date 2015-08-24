/* simplicial_complex.cpp
 *
 * Rui Tong
 * Department of Computer Science
 * San Jose State University
 *
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stack>
#include <unistd.h>
using namespace std;

/* Vertex data structure
 *
 * Description: The structure Star represents each Star-neighborhood in the simplicial complex.
 * Vertex->connections points to a 2-Dimension bool array, which has all the connections among vertexes
 * Vertex->vertex_name is an integer array has all the vertex name related to core vertex
 * Vertex->support is the support for each vertex
 * Vertex->num_simplex is number of the connections, each connection is kind of a close simplex
 * Vertex->num_vertex is number of the vertexes in this Star
 * Vertex->core_simplex is the name for current core simplex
 * Vertex->count is the core simplex's support
 *
 */
typedef struct Star *Star_pointer;	/* Point to Star structure */
typedef struct Star {
	bool ** connections; //all the connections among vertexes
	int * vertex_name; //all vertex names of this Star
	int * support; // total support of each vertex
	int num_simplex; //number of close simplex of this Star
	int num_vertex; // number of vertex
	string core_simplex; // Star's core simplex's name
	int count; //the support number of core simplex
} Star;

/* Global variables
 * 
 * stack<Star_pointer> stk is used to store temporary star-neighborhood
 * assoc_rule_length is the max length of association rules being mined
 * threshold is the minimum number of support for each association rules
 * num_column is the number of itesm from the original database
 * num_row is the number of connectionss from the original database
 * database[60] is the database name
 * association_rules[60] is file contain all the fianl association rules
 * 
 */
stack<Star_pointer> stk;
int assoc_rule_length;			
int threshold;			
int num_column;			
int num_row;			
char database[60];		
char association_rules[60];

/******************************************************************************************
 * Function: init_connections()
 *
 * Description:
 *	Scan the database and find the support of each 0-simplex (vertex).
 * all the vertexes suppport, no connections yet
 */
int init_connections(Star_pointer tp)
{
	int num_of_connections;
	int vertex_name;
	int maxSize=0;
	FILE *fp;
	int i, j;

	tp->connections = new bool*[num_row];
	for(i=0;i<num_row;i++)
	{
		tp->connections[i] = new bool[num_column];
		memset(tp->connections[i], 0, num_column);
	}

	tp->support = new int[num_column];
	//memset function, the third parameter is based on unsigned char
	memset(tp->support, 0, num_column*4);
   
	tp->vertex_name = new int[num_column];
	for(i=0;i<num_column;i++)
		tp->vertex_name[i] = i;

	tp->num_simplex = num_row;
	tp->num_vertex = num_column;
	tp->count = 0;
	tp->core_simplex = "";

	if ((fp = fopen(database, "r")) == NULL) {
		printf("Can't open the file, %s.\n", database);
		exit(1);
	}

	//visit every connection in the Database
	for (i=0; i < num_row; i++) {
		fscanf(fp, "%d", &num_of_connections);

		if (num_of_connections > maxSize)
		maxSize = num_of_connections;

		//read the vertexes in every connection
		for (j=0; j < num_of_connections; j++) {
			fscanf(fp, "%d", &vertex_name);
			tp->support[vertex_name]++;
			tp->connections[i][vertex_name]=1;
		}
	} 
	fclose(fp);

	return 0;
}

/******************************************************************************************
 * Function: read_config_file()
 *
 * Description:
 *	Read the read_config_file parameters from the configuration file.
 */
int read_config_file(char *configFile)
{
	FILE *fp;
	float thresholdDecimal;
	if ((fp = fopen(configFile, "r")) == NULL) {
		printf("Can't open config file, %s.\n", configFile);
		exit(1);
	}
	fscanf(fp, "%d %f %d %d", &assoc_rule_length, &thresholdDecimal, &num_column, &num_row);
	fscanf(fp, "%s %s", database, association_rules);
	fclose(fp);

	threshold = thresholdDecimal * num_row;
	if (threshold == 0) threshold = 1;

	return 0;
}
/******************************************************************************************
 * Function: show_time()
 * Description: show the running time start from beginning
 */
double show_time(struct timespec *start)
{
	struct timespec finish;
	double elapsed;
	
	clock_gettime(CLOCK_MONOTONIC, &finish);

	elapsed = (finish.tv_sec - start->tv_sec);
	elapsed += (finish.tv_nsec - start->tv_nsec) / 1000000000.0;
}
/* 
 * clock() measure CPU time, not Wall time, so it won't work in multithread program.
 *
 * float show_time(){
 *       float time=(float)clock()/CLOCKS_PER_SEC;
 *       //printf("time %.4f secs.\n", time);
 *       return time;
 * }
 *
 */

/******************************************************************************************
 * Function: destroy()
 * Description: free the space allocated for the Vertex struction
 */
void destroy(Star_pointer tp)
{
	for( int i = 0 ; i < tp->num_simplex ; i++ )
		delete [] tp->connections[i];
	delete [] tp->connections;

	delete [] tp->vertex_name;
	delete [] tp->support;

	delete tp;
}
/******************************************************************************************
 *Function: check_related_vertex()
 *
 *Description: check and keep the vertex has connections >= threshold
 * copy and delete the previous one
 */
Star_pointer check_related_vertex(Star_pointer tp)
{
	Star_pointer p = new Star;
	int ncolumn = 0;//new num_vertex
	for(int i=0;i<tp->num_vertex;i++)
	{
		if(tp->support[i]>=threshold)
			ncolumn++;
	}
	p->num_vertex = ncolumn;
	p->num_simplex = tp->num_simplex;
	p->core_simplex = tp->core_simplex;
	p->count = tp->count;

	p->vertex_name = new int [p->num_vertex];//new vertex_name array
	p->support = new int [p->num_vertex];//new support array

	p->connections = new bool*[p->num_simplex];
	if(p->connections == 0){
		cout<<"out of memory"<<endl;
		exit(0);
	}
	for(int i=0;i<p->num_simplex;i++){
		p->connections[i] = new bool[p->num_vertex];
		if(p->connections[i] == 0){
		cout<<"out of memory"<<endl;
		exit(0);
		}
	}

	int taken_row = 0;//used to iterate new array
	for(int i=0;i<tp->num_vertex;i++)//copy into new 2-d array p[][]
	{
		if(tp->support[i]>=threshold)
		{
			p->support[taken_row] = tp->support[i];//copy new support
			p->vertex_name[taken_row] = tp->vertex_name[i];//copy new vertex_name
			for(int j= 0;j<tp->num_simplex;j++)
				p->connections[j][taken_row] = tp->connections[j][i];//copy 0 or 1
			taken_row++;
		}
	}

	/*delete previous connections*/
	destroy(tp);
	
	return p;
}
/******************************************************************************************
 * Function: exchange()
 *
 * Description: This function is used to exchange the vertex_name according to the support
 * 	
 */
int exchange(int *support, int *vertex_nameset, int x, int i)
{ 
	int temp; 
	//here is to exchange the support
	temp = support[x];
	support[x] = support[i];
	support[i] = temp;

	//here is to exchange the vertex_name
	temp = vertex_nameset[x];
	vertex_nameset[x] = vertex_nameset[i];
	vertex_nameset[i] = temp;
 
	return 0;
}

/******************************************************************************************
 * Function: sort_two_array_asec()
 *
 * Description: this function will sort the two related array in asec order
 * 	
 */
int sort_two_array_asec(int *support, int *vertex_nameset, int low,int high, int size)
{
	int pass;
	int highptr=high++;    
	int pivot=low;

	if(low>=highptr) return 0;
	do {
	pass=1;
	while(pass==1) {
		if(++low<size) {
			if(support[low] < support[pivot])
				pass=1;
			else pass=0;
		} else pass=0;
	} 

	pass=1; 
	while(pass==1) {
		if(high-->0) { 
			if(support[high] > support[pivot]) 
				pass=1;
			else pass=0; 
		} else pass=0; 
	}

	if(low<high)
		exchange(support, vertex_nameset, low, high);
	} while(low<=high);

	exchange(support, vertex_nameset, pivot, high);

	sort_two_array_asec(support, vertex_nameset, pivot, high-1, size); 
	sort_two_array_asec(support, vertex_nameset, high+1, highptr, size);

	return 0;
}
/******************************************************************************************
 * Function: extract_Star()
 *
 * Description: extract Star according to their support in ascending order. Every Star 
 *		contains all the connections among vertexes in it. For example, if we have a pyramid, 
 *		then we start from the vertex on the top of the pyramid, take out all the connections 
 *		together with this vertex. 
 *
 */

void extract_Star(Star_pointer tp, stack<Star_pointer> **sp, int num_thread)
{
	ofstream myfile;
	myfile.open(association_rules);
	for(int i=0;i<tp->num_vertex;i++)
		myfile<<"  "<<tp->vertex_name[i]<<" ["<<tp->support[i]<<"]"<<endl;

	int * sorted_index = (int *)malloc(sizeof(int) * tp->num_vertex);
	int * tmp_support = (int *)malloc(sizeof(int) * tp->num_vertex);
	for(int v=0;v<tp->num_vertex;v++)
		sorted_index[v] = v;

	for(int v=0;v<tp->num_vertex;v++)
		tmp_support[v] = tp->support[v];

	sort_two_array_asec(tmp_support, sorted_index, 0, tp->num_vertex -1 , tp->num_vertex);

	/*extract Star from least support of vertex*/
	for(int i=0;i<tp->num_vertex-1;i++)//i loop through each vertex_name in connections
	{
		int pi = sorted_index[i];//the sequence index, pi is the index in sorted_index[]

		string s = "";
		stringstream out;
		out << tp->vertex_name[pi];//get the string version of vertex_name
		s = out.str();
		Star_pointer stakp = new Star;//new sub-connections

		/*initial value*/
		stakp->num_vertex = 0;
		stakp->num_simplex = 0;
		stakp->count = 0;
		stakp->vertex_name = 0;
		stakp->core_simplex = "";
		stakp->support = 0;
		stakp->connections = 0;

		/*column and row*/
		stakp->num_vertex = tp->num_vertex-i-1;//sub connections column
		stakp->num_simplex = tp->support[pi];

		stakp->connections = new bool*[stakp->num_simplex];//support num is the row num of new sub connections
		for(int k=0;k<stakp->num_simplex;k++)
			stakp->connections[k] = new bool[stakp->num_vertex];

		/*vertex_name, support, string and count*/
		stakp->vertex_name = new int[stakp->num_vertex];
		stakp->support = new int[stakp->num_vertex];
		memset(stakp->support,0,stakp->num_vertex*sizeof(int));

		/*sub connections*/
		int m =0;
		for(int j=0;j<tp->num_simplex;j++)//j loop through row to find 1
		{
			if(tp->connections[j][pi] == 1){
				int mm =0;//mm is used to increment support index
				int si; //sorted_index index
				for(int n=i+1;n<tp->num_vertex;n++)//n = i+1
				{
					si = sorted_index[n];
						bool tmp = tp->connections[j][si]; 
						stakp->connections[m][mm] = tmp;
						if(tmp == 1)
							stakp->support[mm]++;
						mm++;
				}
				m++;
			}
		}

		/*vertex_name, string, count*/
		m = 0;
		int si = 0;
		for(int n=i+1;n<tp->num_vertex;n++)
		{
			si = sorted_index[n];
				stakp->vertex_name[m] = tp->vertex_name[si];
				m++;
		}
		stakp->core_simplex = tp->core_simplex +" "+ s;
		stakp->count = tp->support[pi];
		
		/*shrink by threshold*/
		Star_pointer newp = check_related_vertex(stakp);

		if(newp->num_vertex >0){
			sp[i%num_thread]->push(newp);
		}
	}
	destroy(tp);
}

/******************************************************************************************
 * Function: visit_Star()
 *
 * Description: visit every Star structure. Start from the core vertex of Star, connect with 
 *			all the other vertex to form a high dimension simplex, until no possible simplex 
 *			exist in the Star.
 * 1. output the frequent patterns based on support
 * 2. if current Vertex has two or more related vertex, then check the lines, triangles and so on.
 *
 */

void *visit_Star(void *stack_pointer){
	stack<Star_pointer> *sp = (stack<Star_pointer> *)stack_pointer;

        char *filename = (char *)malloc(10); //need to free it in main function!
        strcpy(filename, "tmpXXXXXX");
        mkstemp(filename); 

	ofstream myfile;
	myfile.open(filename, std::fstream::app);

	while(!sp->empty())//loop through the stack until it's empty
	{
		Star_pointer tp = sp->top();//get the first one everytime

		sp->pop();//then delete it from stack

		//output current connections's frequent patterns 
		for(int i=0;i<tp->num_vertex;i++){
			if(tp->support[i] >= threshold){
				string s = "";
				stringstream out;
				out << tp->vertex_name[i];//get the string version of vertex_name
				s = out.str();
				myfile<<" "<<tp->core_simplex+" "+s<<" ["<<((tp->count<tp->support[i])?tp->count:tp->support[i])<<"]"<<endl;
			}
		}
		//
		if(tp->num_vertex >1)//don't need to do only one column table
		{
			int * sorted_index = (int *)malloc(sizeof(int) * tp->num_vertex);
			int * tmp_support = (int *)malloc(sizeof(int) * tp->num_vertex);
			for(int v=0;v<tp->num_vertex;v++)
				sorted_index[v] = v;

			for(int v=0;v<tp->num_vertex;v++)
				tmp_support[v] = tp->support[v];

			sort_two_array_asec(tmp_support, sorted_index, 0, tp->num_vertex -1 , tp->num_vertex);

			for(int i=0;i<tp->num_vertex-1;i++)//i loop through each vertex_name in connections
			{
				int pi = sorted_index[i];//the sequence index, pi is the index in sorted_index[]

				string s = "";
				stringstream out;
				out << tp->vertex_name[pi];//get the string version of vertex_name
				s = out.str();
				Star_pointer stakp = new Star;//new sub-connections

				/*initial value*/
				stakp->num_vertex = 0;
				stakp->num_simplex = 0;
				stakp->count = 0;
				stakp->vertex_name = 0;
				stakp->core_simplex = "";
				stakp->support = 0;
				stakp->connections = 0;

				/*column and row*/
				stakp->num_vertex = tp->num_vertex-i-1;//sub connections column
				stakp->num_simplex = tp->support[pi];

				stakp->connections = new bool*[stakp->num_simplex];//support num is the row num of new sub connections
				for(int k=0;k<stakp->num_simplex;k++)
					stakp->connections[k] = new bool[stakp->num_vertex];

				/*vertex_name, support, string and count*/
				stakp->vertex_name = new int[stakp->num_vertex];
				stakp->support = new int[stakp->num_vertex];
				memset(stakp->support,0,stakp->num_vertex*sizeof(int));

				/*sub connections*/
				int m =0;
				for(int j=0;j<tp->num_simplex;j++)//j loop through row to find 1
				{
					if(tp->connections[j][pi] == 1){
						int mm =0;//mm is used to increment support index
						int si; //sorted_index index
						for(int n=i+1;n<tp->num_vertex;n++)//n = i+1
						{
							si = sorted_index[n];
								bool tmp = tp->connections[j][si]; 
								stakp->connections[m][mm] = tmp;
								if(tmp == 1)
									stakp->support[mm]++;
								mm++;
						}
						m++;
					}
				}

				/*vertex_name, string, count*/
				m = 0;
				int si = 0;
				for(int n=i+1;n<tp->num_vertex;n++)
				{
					si = sorted_index[n];
						stakp->vertex_name[m] = tp->vertex_name[si];
						m++;
				}
				stakp->core_simplex = tp->core_simplex +" "+ s;
				stakp->count = tp->support[pi];

				/*shrink by threshold*/
				Star_pointer newp = check_related_vertex(stakp);

				if(newp->num_vertex >0)
					sp->push(newp);
			}
			destroy(tp);
		}

	}
	return (void *)filename;
}
/******************************************************************************************
 * Function: main
 */
int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("Usage: %s <config-file> <#-of-thread>\n\n", argv[0]);
		exit(1);
	}

	int num_thread = atoi(argv[2]), i;
	printf("\nrunning on %d thread(s) ...\n", num_thread);
	
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	double time1 = show_time(&start);//beginning
	read_config_file(argv[1]);

	Star_pointer op = new Star;
	init_connections(op);
	Star_pointer tp =  check_related_vertex(op);//trim not qualified vertex

	double time2 = show_time(&start);//constructing
	printf("Initialization: \t%.3f seconds\n", time2 - time1);

	stack<Star_pointer> **stackp = (stack<Star_pointer> **)malloc(num_thread * sizeof(stack<Star_pointer> *));
	for(i=0;i<num_thread;i++)
		stackp[i] = new stack<Star_pointer>;

	extract_Star(tp, stackp, num_thread);
	double time3 = show_time(&start);//extract
	printf("Building Stacks: \t%.3f seconds\n", time3 - time2);

	pthread_t *t = (pthread_t *)malloc(num_thread * sizeof(pthread_t));
        for(i=0;i<num_thread;i++){
                pthread_create(&t[i], NULL, visit_Star, (void *)stackp[i]);
        }

	/*merge tmp files into result file*/	
	for(i=0;i<num_thread;i++){
		void *res;
		pthread_join(t[i], &res);

		ofstream fout(association_rules, std::ofstream::app);
		ifstream fin((char *)res);
		string line;
		while(std::getline(fin, line)) 
			fout << line << '\n';
		
		unlink((char*)res);
		free((char*)res);
	}
	
	double time4 = show_time(&start);
	printf("Frequent Patterns: \t%.3f seconds\n\n", time4 - time3);

	/*free the stackp*/
	for(i=0;i<num_thread;i++){
		delete stackp[i];
	}
	free(stackp);
	free(t);

	return 0;
}

