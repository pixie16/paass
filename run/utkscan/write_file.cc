#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int main(int argc, char *argv[]){
 if (argc < 2 ) {
  std::cout << "Usage: ./write_file [cmd filename] [ldf file string] [# of ldf files]" << endl;
  return 0;
  }

char *file_name= argv[1]; 
char *ldf_file = argv[2];
int n_files = atoi(argv[3]);

  //strcat(file_name,".cmd");

  FILE *out;
  out = fopen(file_name,"w");

    cout<<"cmd file "<<file_name<<" generated successfully"<<endl;
    
    for (int i=0;i<n_files;i++){
      char full_name[256];
      if(i==0)
	sprintf(full_name,"../data/%s.ldf",ldf_file);
      else
	sprintf(full_name,"../data/%s-%d.ldf",ldf_file,i);
      fprintf(out,"file %s\n",full_name);
      fprintf(out,"run \nsync\n");
    }
    fprintf(out,"quit\n");
    return 0;
  
}

