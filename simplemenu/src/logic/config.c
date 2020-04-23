#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../headers/definitions.h"
#include "../headers/globals.h"
#include "../headers/logic.h"
#include "../headers/string_utils.h"
#include "../headers/ini2.h"
#include "../headers/graphics.h"

char home[5000];
char pathToThemeConfigFile[1000];
char pathToThemeConfigFilePlusFileName[1000];

void loadAliasList(int sectionNumber) {
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	FILE *aliasFile=getCurrentSectionAliasFile();
	if (aliasFile==NULL) {
		strcpy(menuSections[sectionNumber].aliasFileName," ");
		return;
	}
	menuSections[sectionNumber].aliasHashTable = ht_create(MAX_GAMES_IN_SECTION);
	while ((read = getline(&line, &len, aliasFile)) != -1) {
		char *romName = strtok(line, "=");
		char *alias = strtok(NULL, "=");
		int lastChar = strlen(alias)-1;
		alias[lastChar]='\0';
		ht_set(menuSections[sectionNumber].aliasHashTable, romName, alias);
	}
	if (line) {
		free(line);
	}
	fclose(aliasFile);
}

void checkIfDefault() {
	FILE *fp=NULL;
	FILE *fpScripts=NULL;
	#ifdef TARGET_BITTBOY
	fp = fopen("/mnt/autoexec.sh", "r");
	fpScripts = fopen("scripts/autoexec.sh", "r");
	#endif
	#ifdef TARGET_RG300
	fp = fopen("/home/retrofw/autoexec.sh", "r");
	fpScripts = fopen("scripts/autoexec.sh", "r");
	#endif
	#ifdef TARGET_RG350
	fp = fopen("/media/data/local/sbin/frontend_start", "r");
	fpScripts = fopen("scripts/frontend_start", "r");
	#endif
	shutDownEnabled=1;
	int sameFile=1;
	int c1, c2;
	if (fp==NULL) {
		shutDownEnabled=0;
		return;
	}
	c1 = getc(fp);
	c2 = getc(fpScripts);
	while (sameFile && c1 != EOF && c2 != EOF) {
		if (c1 != c2) {
			sameFile=0;
			shutDownEnabled=0;
			break;
		}
		c1 = getc(fp);
		c2 = getc(fpScripts);
	}
	if (fp!=NULL) {
		fclose(fp);
	}
	if (fpScripts!=NULL) {
		fclose(fpScripts);
	}
}

uint32_t hex2int(char *hex) {
	char *hexCopy = malloc(strlen(hex)+1);
	strcpy(hexCopy,hex);
	uint32_t val = 0;
	while (*hexCopy) {
		// get current character then increment
		uint8_t byte = *hexCopy++;
		// transform hex character to the 4bit equivalent number, using the ascii table indexes
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
		// shift 4 to make space for new digit, and add the 4 bits of the new digit
		val = (val << 4) | (byte & 0xF);
	}
	return val;
}

void setRGBFromHex (int *rgbColor, const char *hexColor) {
	char r[3];
	char g[3];
	char b[3];
	r[0]=hexColor[0];r[1]=hexColor[1];r[2]='\0';g[0]=hexColor[2];g[1]=hexColor[3];g[2]='\0';b[0]=hexColor[4];b[1]=hexColor[5];b[2]='\0';
	rgbColor[0]=hex2int(r);
	rgbColor[1]=hex2int(g);
	rgbColor[2]=hex2int(b);
}


void setStringValueInSection(ini_t *config, char *sectionName, char *valueName, char *sectionValueToBeSet) {
	const char *value;
	value = ini_get(config, sectionName, valueName);
	if(value==NULL) {
		strcpy(sectionValueToBeSet,"\0");
		return;
	}
	strcpy(sectionValueToBeSet,value);
}

void setRGBColorInSection(ini_t *config, char *sectionName, char *valueName, int *sectionValueToBeSet) {
	const char *value;
	value = ini_get(config, sectionName, valueName);
	if (value==NULL) {
		value = ini_get(config, "DEFAULT", valueName);
	}
	setRGBFromHex(sectionValueToBeSet, value);
}

void setThemeResourceValueInSection(ini_t *config, char *sectionName, char *valueName, char *sectionValueToBeSet) {
	const char *value;
	value = ini_get(config, sectionName, valueName);
	if(value==NULL) {
		strcpy(sectionValueToBeSet,"\0");
		return;
	}
	strcpy(sectionValueToBeSet,pathToThemeConfigFile);
	strcat(sectionValueToBeSet,value);
}


void loadTheme(char *theme) {
	strcpy(pathToThemeConfigFilePlusFileName,theme);
	char *temp = getRomPath(theme);
	strcat(temp,"/");
	strcpy(pathToThemeConfigFile,temp);
	free(temp);
	ini_t *themeConfig = ini_load(theme);
	for (int i=0;i<menuSectionCounter;i++) {
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "headerBackGround", menuSections[i].headerAndFooterBackgroundColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "headerFont", menuSections[i].headerAndFooterTextColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "bodyBackground", menuSections[i].bodyBackgroundColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "bodyFont", menuSections[i].bodyTextColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "selectedItemBackground", menuSections[i].bodySelectedTextBackgroundColor);
		setRGBColorInSection(themeConfig, menuSections[i].sectionName, "selectedItemFont", menuSections[i].bodySelectedTextTextColor);
		setThemeResourceValueInSection (themeConfig, menuSections[i].sectionName, "logo", menuSections[i].systemLogo);
		setThemeResourceValueInSection (themeConfig, menuSections[i].sectionName, "system", menuSections[i].systemPicture);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "menu_mode_logo_background", simpleBackground);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "fullscreen_background", fullscreenBackground);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "no_pic", nopic);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "font", menuFont);
		freeFonts();
		initializeFonts();
	}
}

void checkThemes() {
	char *files[1000];
	char tempString[1000];
	snprintf(tempString,sizeof(tempString),"%s/.simplemenu/themes/",getenv("HOME"));
	int n = findDirectoriesInDirectory(tempString, files, 0);
	qsort(files, n, sizeof(const char*), sortStringArray);
	for(int i=0;i<n;i++) {
		themes[i]=malloc(strlen(files[i])+1);
		strcpy(themes[i],files[i]);
		themeCounter++;
	}
}

void createConfigFilesInHomeIfTheyDontExist() {
	snprintf(home,sizeof(home),"%s",getenv("HOME"));
	char pathToConfigFiles[3000];
	char pathToAppFiles[3000];
	char pathToGameFiles[3000];
	char pathToThemeFiles[3000];
	char pathToSectionGroupsFiles[3000];
	snprintf(pathToConfigFiles,sizeof(pathToConfigFiles),"%s/.simplemenu",home);
	snprintf(pathToAppFiles,sizeof(pathToConfigFiles),"%s/.simplemenu/apps",home);
	snprintf(pathToGameFiles,sizeof(pathToGameFiles),"%s/.simplemenu/games",home);
	snprintf(pathToSectionGroupsFiles,sizeof(pathToSectionGroupsFiles),"%s/.simplemenu/section_groups",home);
	snprintf(pathToThemeFiles,sizeof(pathToThemeFiles),"%s/.simplemenu/themes",home);
	int directoryExists=mkdir(pathToConfigFiles,0700);
	if (!directoryExists) {
		char copyCommand[3000];
		snprintf(copyCommand,sizeof(copyCommand),"cp ./config/* %s/.simplemenu/",home);
		int ret = system(copyCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		char copyAppsCommand[3000];
		mkdir(pathToAppFiles,0700);
		snprintf(copyAppsCommand,sizeof(copyAppsCommand),"cp ./apps/* %s/.simplemenu/apps",home);
		ret = system(copyAppsCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		char copyGamesCommand[3000];
		mkdir(pathToGameFiles,0700);
		snprintf(copyGamesCommand,sizeof(copyGamesCommand),"cp ./games/* %s/.simplemenu/games",home);
		ret = system(copyGamesCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		char copyThemesCommand[3000];
		mkdir(pathToThemeFiles,0700);
		snprintf(copyThemesCommand,sizeof(copyThemesCommand),"cp -r ./themes/* %s/.simplemenu/themes",home);
		ret = system(copyThemesCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
		char copySectionGroupsCommand[3000];
		mkdir(pathToSectionGroupsFiles,0700);
		snprintf(copySectionGroupsCommand,sizeof(copySectionGroupsCommand),"cp -r ./section_groups/* %s/.simplemenu/section_groups",home);
		ret = system(copySectionGroupsCommand);
		if (ret==-1) {
			generateError("FATAL ERROR", 1);
		}
//		char deleteDirectoriesCommand[3000];
//		snprintf(deleteDirectoriesCommand,sizeof(deleteDirectoriesCommand),"rm -rf ./apps");
//		ret = system(deleteDirectoriesCommand);
//		snprintf(deleteDirectoriesCommand,sizeof(deleteDirectoriesCommand),"rm -rf ./games");
//		ret = system(deleteDirectoriesCommand);
//		snprintf(deleteDirectoriesCommand,sizeof(deleteDirectoriesCommand),"rm -rf ./themes");
//		ret = system(deleteDirectoriesCommand);
//		snprintf(deleteDirectoriesCommand,sizeof(deleteDirectoriesCommand),"rm -rf ./config");
//		ret = system(deleteDirectoriesCommand);
	}
}

void saveFavorites() {
	if (favoritesChanged) {
		FILE * fp;
		char pathToFavoritesFilePlusFileName[300];
		snprintf(pathToFavoritesFilePlusFileName,sizeof(pathToFavoritesFilePlusFileName),"%s/.simplemenu/favorites.sav",home);
		fp = fopen(pathToFavoritesFilePlusFileName, "w");
		int linesWritten=0;
		for (int j=0;j<favoritesSize;j++) {
			struct Favorite favorite = favorites[j];
			if (strlen(favorite.name)==1) {
				break;
			}
			if(linesWritten>0) {
				fprintf(fp,"\n");
			}
//			favorite=findFavorite(favorite.alias);
			fprintf(fp,"%s;",favorite.section);
			fprintf(fp,"%s;",favorite.name);
			if(favorite.alias[0]=='\0') {
				fprintf(fp," ;");
			} else {
				fprintf(fp,"%s;",favorite.alias);
			}
			fprintf(fp,"%s;",favorite.emulatorFolder);
			fprintf(fp,"%s;",favorite.executable);
			fprintf(fp,"%s",favorite.filesDirectory);
			linesWritten++;
		}
		fclose(fp);
		favoritesChanged=0;
	}
}

void loadFavorites() {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char pathToFavoritesFilePlusFileName[300];
	snprintf(pathToFavoritesFilePlusFileName,sizeof(pathToFavoritesFilePlusFileName),"%s/.simplemenu/favorites.sav",home);
	fp = fopen(pathToFavoritesFilePlusFileName, "r");
	if (fp==NULL) {
		generateError("FAVORITES FILE NOT FOUND-SHUTTING DOWN",1);
		return;
	}
	char *configurations[6];
	char *ptr;
	favoritesSize=0;
	while ((read = getline(&line, &len, fp)) != -1) {
		ptr = strtok(line, ";");
		int i=0;
		while(ptr != NULL) {
			configurations[i]=ptr;
			ptr = strtok(NULL, ";");
			i++;
		}
		strcpy(favorites[favoritesSize].section,configurations[0]);
		strcpy(favorites[favoritesSize].name,configurations[1]);
		strcpy(favorites[favoritesSize].alias,configurations[2]);
		strcpy(favorites[favoritesSize].emulatorFolder,configurations[3]);
		strcpy(favorites[favoritesSize].executable,configurations[4]);
		strcpy(favorites[favoritesSize].filesDirectory,configurations[5]);
		int len = strlen(favorites[favoritesSize].filesDirectory);
		if (favorites[favoritesSize].filesDirectory[len-1]=='\n') {
			favorites[favoritesSize].filesDirectory[len-1]='\0';
		}
		favoritesSize++;
	}
	fclose(fp);
	if (line) {
		free(line);
	}
	qsort(favorites, favoritesSize, sizeof(struct Favorite), compareFavorites);
}

int cmpfnc(const void *f1, const void *f2)
{
	struct SectionGroup *e1 = (struct SectionGroup *)f1;
	struct SectionGroup *e2 = (struct SectionGroup *)f2;
	char temp1[300]="";
	char temp2[300]="";
	strcpy(temp1,e1->groupName);
	strcpy(temp2,e2->groupName);
	for(int i=0;temp1[i]; i++) {
		temp1[i] = tolower(temp1[i]);
	}
	for(int i=0;temp2[i]; i++) {
		temp2[i] = tolower(temp2[i]);
	}
	return strcmp(temp1, temp2);
}


void loadConfig() {
//	snprintf(home,sizeof(home),"%s",getenv("HOME"));
	char pathToConfigFilePlusFileName[1000];
	const char *value;
	snprintf(pathToConfigFilePlusFileName,sizeof(pathToConfigFilePlusFileName),"%s/.simplemenu/config.ini",home);
	ini_t *config = ini_load(pathToConfigFilePlusFileName);
	value = ini_get(config, "GENERAL", "theme");
	snprintf(pathToThemeConfigFile,sizeof(pathToThemeConfigFile),"%s/.simplemenu/",home);
	strcat(pathToThemeConfigFile, "themes/");
	strcat(pathToThemeConfigFile, value);
	strcat(pathToThemeConfigFile, "/");
	strcpy(pathToThemeConfigFilePlusFileName, pathToThemeConfigFile);
	strcat(pathToThemeConfigFilePlusFileName, "theme.ini");

	ini_t *themeConfig = ini_load(pathToThemeConfigFilePlusFileName);

	value = ini_get(themeConfig, "GENERAL", "menu_mode_logo_background");
	snprintf(simpleBackground,sizeof(simpleBackground),"%s%s",pathToThemeConfigFile, value);

	value = ini_get(themeConfig, "GENERAL", "fullscreen_background");
	snprintf(fullscreenBackground,sizeof(fullscreenBackground),"%s%s",pathToThemeConfigFile, value);

	value = ini_get(themeConfig, "GENERAL", "no_pic");
	snprintf(nopic,sizeof(nopic),"%s%s",pathToThemeConfigFile, value);

	value = ini_get(themeConfig, "GENERAL", "font");
	snprintf(menuFont,sizeof(menuFont),"%s%s",pathToThemeConfigFile, value);

	value = ini_get(config, "GENERAL", "media_folder");
	strcpy(mediaFolder,value);

	value = ini_get(config, "CPU", "underclocked_speed");
	OC_UC=atoi(value);

	value = ini_get(config, "CPU", "normal_speed");
	OC_NO=atoi(value);

	value = ini_get(config, "CPU", "overclocked_speed");
	OC_OC=atoi(value);

	value = ini_get(config, "CPU", "sleep_speed");
	OC_SLEEP=atoi(value);

	value = ini_get(config, "SYSTEM", "screen_timeout_in_seconds");
	timeoutValue=atoi(value);

	value = ini_get(config, "SYSTEM", "allow_shutdown");
	shutDownEnabled=atoi(value);

	value = ini_get(config, "FULLSCREEN_MODE", "display_footer");
	footerVisibleInFullscreenMode=atoi(value);

	value = ini_get(config, "FULLSCREEN_MODE", "display_menu");
	menuVisibleInFullscreenMode=atoi(value);

	sectionGroupCounter=0;
	char *files[1000];
	char tempString[1000];
	snprintf(tempString,sizeof(tempString),"%s/.simplemenu/section_groups/",getenv("HOME"));
	int n = recursivelyScanDirectory(tempString, files, 0);

	for(int i=0;i<n;i++) {
		if(strstr(files[i],".png")!=NULL) {
			continue;
		}
		strcpy(sectionGroups[sectionGroupCounter].groupPath, files[i]);
		char *temp = getNameWithoutPath((files[i]));
		char *temp1 = getNameWithoutExtension(temp);

		char *temp3 = malloc(3000);
		strcpy(temp3, getRomPath(files[i]));
		strcat(temp3,"/");
		strcat(temp3,temp1);
		strcat(temp3,".png");
		strcpy(sectionGroups[sectionGroupCounter].groupBackground, temp3);

		char *temp2 = toUpper(temp1);
		strcpy(sectionGroups[sectionGroupCounter].groupName, temp2);
		free(temp);
		free(temp1);
		free(temp2);
		sectionGroupCounter++;
	}
	qsort(sectionGroups, sectionGroupCounter, sizeof(struct SectionGroup), cmpfnc);
}

int loadSections(char *file) {
	menuSectionCounter=0;
	char pathToSectionsFilePlusFileName[1000];
	snprintf(pathToSectionsFilePlusFileName,sizeof(pathToSectionsFilePlusFileName),"%s",file);
	ini_t *config = ini_load(pathToSectionsFilePlusFileName);
	ini_t *themeConfig = ini_load(pathToThemeConfigFilePlusFileName);

	const char *consoles = ini_get(config, "CONSOLES", "consoleList");
	char *consoles1 = strdup(consoles);
	char *sectionNames[10000];
	const char *value;
	int sectionCounter=0;

	char* tokenizedSectionName=strtok(consoles1,",");

	while(tokenizedSectionName!=NULL) {
		value = ini_get(config, tokenizedSectionName, "execs");
		if(value!=NULL) {
			sectionNames[sectionCounter]=malloc(strlen(tokenizedSectionName)+1);
			strcpy(sectionNames[sectionCounter], tokenizedSectionName);
			sectionCounter++;
		}
		tokenizedSectionName=strtok(NULL,",");
	}
	free(tokenizedSectionName);
	while(menuSectionCounter<sectionCounter) {
		char *sectionName = sectionNames[menuSectionCounter];
		strcpy(menuSections[menuSectionCounter].sectionName, sectionName);


		//READ EXECS
		int execCounter=0;
		value = ini_get(config, sectionName, "execs");
		char *value2 = malloc(3000);
		strcpy(value2,value);
		for (int i=0;i<10;i++) {
			menuSections[menuSectionCounter].executables[i]=NULL;
		}
		char* currentExec = strtok(value2,",");
		while(currentExec!=NULL) {
			char *tempNameWithoutPath = getNameWithoutPath(currentExec);
			char *tempPathWithoutName = getRomPath(currentExec);
			menuSections[menuSectionCounter].executables[execCounter]=malloc(strlen(tempNameWithoutPath)+1);
			strcpy(menuSections[menuSectionCounter].executables[execCounter],tempNameWithoutPath);
			free(tempNameWithoutPath);
			menuSections[menuSectionCounter].emulatorDirectories[execCounter]=malloc(strlen(tempPathWithoutName)+2);
			strcpy(menuSections[menuSectionCounter].emulatorDirectories[execCounter],tempPathWithoutName);
			free(tempPathWithoutName);
			strcat(menuSections[menuSectionCounter].emulatorDirectories[execCounter],"/");
			strcat(menuSections[menuSectionCounter].emulatorDirectories[execCounter],"\0");
			currentExec = strtok(NULL,",");
			execCounter++;
		}
		free (currentExec);
		menuSections[menuSectionCounter].executables[execCounter]=NULL;
		menuSections[menuSectionCounter].activeExecutable=0;

		setStringValueInSection (config, sectionName, "romDirs", menuSections[menuSectionCounter].filesDirectories);
		setStringValueInSection (config, sectionName, "romExts", menuSections[menuSectionCounter].fileExtensions);
		setRGBColorInSection(themeConfig, sectionName, "headerBackGround", menuSections[menuSectionCounter].headerAndFooterBackgroundColor);
		setRGBColorInSection(themeConfig, sectionName, "headerFont", menuSections[menuSectionCounter].headerAndFooterTextColor);
		setRGBColorInSection(themeConfig, sectionName, "bodyBackground", menuSections[menuSectionCounter].bodyBackgroundColor);
		setRGBColorInSection(themeConfig, sectionName, "bodyFont", menuSections[menuSectionCounter].bodyTextColor);
		setRGBColorInSection(themeConfig, sectionName, "selectedItemBackground", menuSections[menuSectionCounter].bodySelectedTextBackgroundColor);
		setRGBColorInSection(themeConfig, sectionName, "selectedItemFont", menuSections[menuSectionCounter].bodySelectedTextTextColor);
		setThemeResourceValueInSection (themeConfig, sectionName, "logo", menuSections[menuSectionCounter].systemLogo);
		setThemeResourceValueInSection (themeConfig, sectionName, "system", menuSections[menuSectionCounter].systemPicture);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "menu_mode_logo_background", simpleBackground);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "fullscreen_background", fullscreenBackground);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "no_pic", nopic);
		setThemeResourceValueInSection (themeConfig, "GENERAL", "font", menuFont);
		freeFonts();
		initializeFonts();
		value = ini_get(config, sectionName, "aliasFile");
		if(value!=NULL) {
			strcpy(menuSections[menuSectionCounter].aliasFileName,value);
		} else {
			strcpy(menuSections[menuSectionCounter].aliasFileName,"\0");
		}

		setStringValueInSection (config, sectionName, "category", menuSections[menuSectionCounter].category);

		value = ini_get(config, sectionName, "onlyFileNamesNoPathOrExtension");
		if(value!=NULL&&strcmp("yes",value)==0) {
			menuSections[menuSectionCounter].onlyFileNamesNoExtension=1;
		}
		menuSections[menuSectionCounter].currentPage=0;
		menuSections[menuSectionCounter].currentGameInPage=0;
		menuSectionCounter++;
	}
	strcpy(menuSections[menuSectionCounter].sectionName,"FAVORITES");
	menuSections[menuSectionCounter].emulatorDirectories[0]=malloc(strlen("/some/folder/")+1);
	strcpy(menuSections[menuSectionCounter].emulatorDirectories[0],"/some/folder/");
	strcat(menuSections[menuSectionCounter].emulatorDirectories[0],"\0");
	menuSections[menuSectionCounter].activeEmulatorDirectory=0;
	menuSections[menuSectionCounter].executables[0]=NULL;
	menuSections[menuSectionCounter].executables[0]=malloc(strlen("favs")+1);
	strcpy(menuSections[menuSectionCounter].executables[0],"favs");
	menuSections[menuSectionCounter].activeExecutable=0;
	strcpy(menuSections[menuSectionCounter].filesDirectories,"/some/folder");
	strcpy(menuSections[menuSectionCounter].fileExtensions,".zzz");
	strcpy(menuSections[menuSectionCounter].category, "all");
	menuSections[menuSectionCounter].onlyFileNamesNoExtension=0;
	menuSections[menuSectionCounter].hidden=0;
	menuSections[menuSectionCounter].currentPage=0;
	menuSections[menuSectionCounter].currentGameInPage=0;
	setRGBColorInSection(themeConfig, "FAVORITES", "headerBackGround", menuSections[menuSectionCounter].headerAndFooterBackgroundColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "headerFont", menuSections[menuSectionCounter].headerAndFooterTextColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "bodyBackground", menuSections[menuSectionCounter].bodyBackgroundColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "bodyFont", menuSections[menuSectionCounter].bodyTextColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "selectedItemBackground", menuSections[menuSectionCounter].bodySelectedTextBackgroundColor);
	setRGBColorInSection(themeConfig, "FAVORITES", "selectedItemFont", menuSections[menuSectionCounter].bodySelectedTextTextColor);
	setThemeResourceValueInSection (themeConfig, "FAVORITES", "logo", menuSections[menuSectionCounter].systemLogo);
	setThemeResourceValueInSection (themeConfig, "FAVORITES", "system", menuSections[menuSectionCounter].systemPicture);

	menuSectionCounter++;
	favoritesSectionNumber=menuSectionCounter-1;
	return menuSectionCounter;
}

int countSections(char *file) {
	menuSectionCounter=0;
	char pathToSectionsFilePlusFileName[1000];
	snprintf(pathToSectionsFilePlusFileName,sizeof(pathToSectionsFilePlusFileName),"%s",file);
	ini_t *config = ini_load(pathToSectionsFilePlusFileName);

	const char *consoles = ini_get(config, "CONSOLES", "consoleList");
	char *consoles1 = strdup(consoles);
	char *sectionNames[10000];
	const char *value;
	int sectionCounter=0;

	char* tokenizedSectionName=strtok(consoles1,",");

	while(tokenizedSectionName!=NULL) {
		value = ini_get(config, tokenizedSectionName, "execs");
		if(value!=NULL) {
			sectionNames[sectionCounter]=malloc(strlen(tokenizedSectionName)+1);
			strcpy(sectionNames[sectionCounter], tokenizedSectionName);
			sectionCounter++;
		}
		tokenizedSectionName=strtok(NULL,",");
	}

	return sectionCounter;
}

void saveLastState() {
	FILE * fp;
	char pathToStatesFilePlusFileName[300];
	snprintf(pathToStatesFilePlusFileName,sizeof(pathToStatesFilePlusFileName),"%s/.simplemenu/last_state.sav",home);
	fp = fopen(pathToStatesFilePlusFileName, "w");
	fprintf(fp, "%d;\n", 54);
	fprintf(fp, "%d;\n", stripGames);
	fprintf(fp, "%d;\n", fullscreenMode);
	fprintf(fp, "%d;\n", footerVisibleInFullscreenMode);
	fprintf(fp, "%d;\n", menuVisibleInFullscreenMode);
	fprintf(fp, "%d;\n", activeTheme);
	fprintf(fp, "%d;\n", timeoutValue);
	fprintf(fp, "%d;\n", autoHideLogos);
	fprintf(fp, "%d;\n", activeGroup);
	fprintf(fp, "%d;\n", currentSectionNumber);
	fprintf(fp, "%d;\n", MENU_ITEMS_PER_PAGE);
	for(int groupCount=0;groupCount<sectionGroupCounter;groupCount++) {
		int sectionsNum=countSections(sectionGroups[groupCount].groupPath);
		for (int sectionCount=0;sectionCount<sectionsNum;sectionCount++) {
			if (groupCount==activeGroup) {
				int isActive = 0;
				if (sectionCount==currentSectionNumber) {
					isActive=1;
				}
				fprintf(fp, "%d;%d;%d;%d;%d\n", isActive, sectionCount, menuSections[sectionCount].currentPage, menuSections[sectionCount].currentGameInPage, menuSections[sectionCount].realCurrentGameNumber);
			} else {
//				loadSections(sectionGroups[groupCount].groupPath);
//				printf("%d;%d;%d;%d;%d\n", sectionGroupStates[groupCount][sectionCount][0], sectionCount, sectionGroupStates[groupCount][sectionCount][1], sectionGroupStates[groupCount][sectionCount][2], sectionGroupStates[groupCount][sectionCount][3]);
				fprintf(fp, "%d;%d;%d;%d;%d\n", sectionGroupStates[groupCount][sectionCount][0], sectionCount, sectionGroupStates[groupCount][sectionCount][1], sectionGroupStates[groupCount][sectionCount][2], sectionGroupStates[groupCount][sectionCount][3]);
			}
		}
	}
	fclose(fp);
}

void loadLastState() {
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	char pathToStatesFilePlusFileName[300];
	snprintf(pathToStatesFilePlusFileName,sizeof(pathToStatesFilePlusFileName),"%s/.simplemenu/last_state.sav",home);
	fp = fopen(pathToStatesFilePlusFileName, "r");
	if (fp==NULL) {
		saveLastState();
		return;
	}
	char *configurations[5];
	char *ptr;
	int startInSection = -1;
	int startInPictureMode = -1;
	int stripGamesConfig = -1;
	int startInGroup= -1;
	int footerVisible= -1;
	int menuVisible= -1;
	int themeRead= -1;
	int timeout= -1;
	int readAutoHideLogos= -1;
	int groupCounter=-1;
	int savedVersion=-1;
	int itemsRead=-1;
	while ((read = getline(&line, &len, fp)) != -1) {
		ptr = strtok(line, ";");
		int i=0;
		while(ptr != NULL) {
			configurations[i]=ptr;
			ptr = strtok(NULL, ";");
			i++;
		}
		if (savedVersion==-1) {
			savedVersion=atoi(configurations[0]);
			if(savedVersion!=53) {
				saveLastState();
				fclose(fp);
				if (line) {
					free(line);
				}
				return;
			}
		} else if (stripGamesConfig==-1) {
			stripGamesConfig=atoi(configurations[0]);
		} else if (startInPictureMode==-1){
			startInPictureMode=atoi(configurations[0]);
		} else if(footerVisible==-1) {
			footerVisible=atoi(configurations[0]);
		} else if(menuVisible==-1) {
			menuVisible=atoi(configurations[0]);
		} else if(themeRead==-1) {
			themeRead=atoi(configurations[0]);
		} else if(timeout==-1) {
			timeout=atoi(configurations[0]);
		} else if(readAutoHideLogos==-1) {
			readAutoHideLogos=atoi(configurations[0]);
		} else if (startInGroup==-1) {
			startInGroup = atoi(configurations[0]);
		} else if (startInSection==-1) {
			startInSection=atoi(configurations[0]);
		} else if (itemsRead==-1) {
			itemsRead=atoi(configurations[0]);
		}
		else {
			if(atoi(configurations[1])==0) {
				groupCounter++;
			}
			int isActive =atoi(configurations[0]);
			int sectionNumber =atoi(configurations[1]);
			int page = atoi(configurations[2]);
			int game = atoi(configurations[3]);
			int realCurrentGameNumber = atoi(configurations[4]);
			sectionGroupStates[groupCounter][sectionNumber][0]=isActive;
			sectionGroupStates[groupCounter][sectionNumber][1]=page;
			sectionGroupStates[groupCounter][sectionNumber][2]=game;
			sectionGroupStates[groupCounter][sectionNumber][3]=realCurrentGameNumber;
			if (groupCounter==startInGroup) {
				menuSections[sectionNumber].currentPage=page;
				menuSections[sectionNumber].currentGameInPage=game;
				menuSections[sectionNumber].realCurrentGameNumber=realCurrentGameNumber;
			}
			menuSections[sectionNumber].alphabeticalPaging=0;
		}
	}
	stripGames=stripGamesConfig;
	fullscreenMode=startInPictureMode;
	footerVisibleInFullscreenMode=footerVisible;
	menuVisibleInFullscreenMode=menuVisible;
	activeTheme=themeRead;
	timeoutValue=timeout;
	autoHideLogos=readAutoHideLogos;
	currentSectionNumber=startInSection;
	activeGroup = startInGroup;
	MENU_ITEMS_PER_PAGE=itemsRead;
	fclose(fp);
	if (line) {
		free(line);
	}
}
