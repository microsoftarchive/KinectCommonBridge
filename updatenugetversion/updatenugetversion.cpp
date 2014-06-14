// updatenugetversion.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include <string.h>

// Prototype
int ParseFile(char *szFilename, int nRank = 3);

// Function ParseFile
// Looks for line containing "version" and increments it, starting at nRank (numbered 0.1.2.3)
// It handles carry in the case of version 9. Note that 9.9.9.9 becomes 0.0.0.0.

int ParseFile(char *szFilename, int nRank)
{
	int nRet = -1;

	const int MAX = 1000;
	char szStr[MAX] = "";

	char szFilename2[MAX];
	sprintf_s(szFilename2, "%s%d", szFilename, 2);

	FILE *fp = NULL;
	FILE *fp2 = NULL;

	fopen_s(&fp, szFilename, "r");
	if (fp)
	{
		fopen_s(&fp2, szFilename2, "w");
		if (fp2)
		{
			char *c = NULL;

			do
			{
				c = fgets(szStr, MAX, fp);

				if (strstr(szStr, "version :") || strstr(szStr, "version:"))
				{
					int v[4] = { -1, -1, -1, -1 };

					sscanf_s(szStr, "\t\tversion : %d.%d.%d.%d;\n", &v[0], &v[1], &v[2], &v[3]);

					bool bDone = false;

					do
					{
						if (v[nRank] < 9)
						{
							v[nRank]++;
							bDone = true;
						}
						else
						{
							v[nRank] = 0;
							nRank--;
						}
					} while (!bDone && nRank > -1);

					fprintf_s(fp2, "\t\tversion : %d.%d.%d.%d;\n", v[0], v[1], v[2], v[3]);

					nRet = nRank;
				}
				else
				{
					if (c)
					{
						fputs(szStr, fp2);
					}
				}
				
			} while (c);
							
			fclose(fp2);
		}

		fclose(fp);
	}

	return nRet;
}

int main(int argc, _TCHAR* argv[])
{
	int nRet = 0;

	if (argc > 1)
	{
		int n = 3;

		if (argc > 2)
		{
			sscanf_s((char *)argv[2], "%d", &n);
		}

		nRet = ParseFile((char *)argv[1], n);
	}

	return nRet;
}

