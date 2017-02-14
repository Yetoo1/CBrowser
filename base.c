/*This is the base for the browser, it doesn't even hold gui, it's a mere module loader*/
#include <stdio.h>
#include <dirent.h>
int main(int argc, char* argv[])
{
	DIR *d;
	struct dirent *dir;
	d = opendir("mod");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			printf("%s\n", dir->d_name);
		}
		closedir(d);
	}
	
	printf("Looking for modules...");
	
}
