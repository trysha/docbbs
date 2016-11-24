/*
 * Deletes deleteable user records, updates sysop and who knows forum lists.
 */
#include "defs.h"
#include "ext.h"

#define LIMIT 26
#undef MARGIN
#define MARGIN 54


struct userinfo
{
  long    usernum;
  char    name[MAXNAME + 1];
  char    generation[MAXROOMS];
  char    forget[MAXROOMS];
  unsigned int f_admin:1;
  unsigned int f_prog:1;
  unsigned int f_aide:1;
};


struct userinfo * getusers();



void
bbsupdate()
{
struct userinfo *start;
long    ucount;
int f;

  setvbuf(stdout, stdoutbuf, _IOFBF, STDOUTBUFSIZ);

  if (!(start = getusers(&ucount)))
  {
    my_printf("Error reading user files\n");
    fflush(stdout);
    _exit(1);
  }

#ifdef _UPDATEHACK
  fflush(stdout);
  _exit(0);
#endif

  if (update_aides(start, &ucount))
  {
    my_printf("Error updating aide list\n");
    fflush(stdout);
    _exit(1);
  }

  if (update_whoknows(start, &ucount))
  {
    my_printf("Error updating whoknowsroom lists\n");
    fflush(stdout);
    _exit(1);
  }

  fflush(stdout);
}



struct userinfo *
getusers(ucount)
long   *ucount;
{
register int i;
struct userinfo *start;
struct user *up;
struct userinfo *uinfo;
struct userdata *ucopy;
long    unbr = 0;
struct tm *ltm;
FILE   *ulog;
int     zap;
int     days_since_on;
int     d;
int     st_tot = 0;
int     st_lold = 0;
int     st_npro = 0;
int     st_nold = 0;
int     st_nov = 0;
int     st_ansi = 0;
FILE   *info;
time_t t;
unsigned char ulogbuf[8192];
unsigned char infobuf[8192];
char	work[80];


  if (!(ucopy = copyuserdata()))
  {
    my_printf ("can't copy userdata\n");
    return(NULL);
  }

  *ucount = ucopy->totalusers[ucopy->which];

  uinfo = (struct userinfo *)mmap(0, *ucount * sizeof(struct userinfo), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  if (!uinfo || uinfo == (struct userinfo *)-1)
  {
    my_printf ("bad uinfo\n");
    return(NULL);
  }
  start = uinfo;
    
  sprintf (work, "%svar/userlist", ROOT);
  if (!(ulog = fopen(work, "w")))
  {
    perror ("open userlist");
    return(NULL);
  }
  setvbuf(ulog, ulogbuf, _IOFBF, sizeof ulogbuf);
  fprintf(ulog, "   date    time   call   post    x       priv  name                 realname               email                                     connection\n");

  sprintf (work, "%svar/info", ROOT);
  if (!(info = fopen(work, "w")))
  {
    perror ("can't open info");
    return(NULL);
  }
  setvbuf(info, infobuf, _IOFBF, sizeof infobuf);

  t = time(0);
  for (d = 0; d < *ucount; d++)
  {
      zap = TRUE;
      if (up = finduser(NULL, ucopy->link[ucopy->name[ucopy->which][d]].usernum, 0))
      {
	zap = FALSE;
	days_since_on = (t - up->time) / 86400;

	/* Zap if new user and hasn't called in 10 days */
	if (days_since_on > 10 && up->f_newbie)
	  zap = TRUE;
	/* Zap if not called in 1 month and called less than 10 x */
	else if (days_since_on > 30 && up->timescalled < 10)
	  zap = TRUE;
	/* Zap if user hasn't called in 120 days */
	else if (days_since_on > 120)
	  zap = TRUE;
	/* Zap if we have marked for deletion */
	else if (up->f_deleted && days_since_on > 1)
	  zap = TRUE;
	/* Don't delete Guest */
	if (!strcmp(up->name, "Guest"))
	  zap = FALSE;
      }
      else
      {
        fprintf(info, "Didn't find user '%s'\n", ucopy->link[ucopy->name[ucopy->which][d]].name);
        continue;
      }

      if (zap)
	deleteuser(up->name);
      else
      {
	uinfo[unbr].usernum = up->usernum;
	strcpy(uinfo[unbr].name, up->name);
	uinfo[unbr].f_admin = up->f_admin;
	uinfo[unbr].f_prog = up->f_prog;
	uinfo[unbr].f_aide = up->f_aide;
	bcopy(up->generation, uinfo[unbr].generation, MAXROOMS);
	bcopy(up->forget, uinfo[unbr].forget, MAXROOMS);
#ifdef _UPDATEHACK
	hack_xconf(up);
#endif
	unbr++;

	st_tot++;
	if (up->f_ansi)
	  st_ansi++;

	ltm = localtime(&up->time);
	i = strcmp(up->A_real_name, up->real_name) || strcmp(up->A_addr1, up->addr1) || strcmp(up->A_city, up->city) || strcmp(up->A_state, up->state) || strcmp(up->A_zip, up->zip) || strcmp(up->A_phone, up->phone) || strcmp(up->A_mail, up->mail);
	fprintf(ulog, "%s%02d%02d%02d  %02d:%02d  %5ld  %6ld  %6ld  0000  %-19s: %-20s : %-40s; %s@%s\n", i ? "** " : "   ", ltm->tm_year, ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, up->timescalled, up->posted, up->totalx, up->name, up->real_name, up->mail, up->loginname, up->remote);
      }

  }
  fprintf(info, "\n\nlold = %d, npro = %d, nold = %d, nov = %d, ansi = %d, tot = %d\n", st_lold, st_npro, st_nold, st_nov, st_ansi, st_tot);
  fclose(info);
  fclose(ulog);
  *ucount = unbr;
  return (start);
}



update_aides(start, ucount)
struct userinfo *start;
long   *ucount;
{
FILE   *af;
int     nameflag = NO;
int     rm_nbr;
char    tmpstr[100];
char    name[100];
struct userinfo *u;
long    unbr;
unsigned char afbuf[8192];

  sprintf(name, "%s.NEW", AIDELIST);
  if (!(af = fopen(name, "w")))
    return (-1);
  setvbuf(af, afbuf, _IOFBF, sizeof afbuf);

  fprintf(af, "Programmers, Sysops, and Forum Moderators\n\n");

  u = start;

  for (unbr = 0; unbr < *ucount; unbr++)
  {
    *tmpstr = 0;

    /* names don't get written to file unless they're aides, etc. */
    if (u->f_admin)
    {
      nameflag = YES;

      sprintf(tmpstr, "\n%s(%ld) ", u->name, u->usernum);
      while (strlen(tmpstr) % LIMIT)
	strcat(tmpstr, " ");

      if (u->f_prog)
	strcat(tmpstr, "programmer ");
      if (u->f_aide)
	strcat(tmpstr, "sysop");

      fprintf(af, "%s\n", tmpstr);
    }
    for (rm_nbr = 0; rm_nbr < MAXROOMS; rm_nbr++)
    {

      if ((u->usernum && u->usernum == msg->room[rm_nbr].roomaide)
	  && (msg->room[rm_nbr].flags & QR_INUSE))
      {
	if (nameflag == NO)
	{
	  nameflag = YES;
	  fprintf(af, "\n%s(%ld) \n", u->name, u->usernum);
	}
	if(!(msg->room[rm_nbr].flags&QR_PRIVATE)){
	/* hide S3KR1T R00MZ */
		fprintf(af, "                         forum moderator for %s\n", msg->room[rm_nbr].name);
	} else {
		fprintf(af, "                         forum moderator for <CENSORED S3KRIT DELETED ROOM %d>\n",rm_nbr);
	}
      }
    }				/* end room loop */
    nameflag = NO;
    u++;
  }				/* end user loop */

  (void)fclose(af);
  rename(name, AIDELIST);
  return (0);
}



update_whoknows(start, ucount)
struct userinfo *start;
int    *ucount;
{
FILE   *file;
char    filestr[160];
char    temp[40];
char    name[100];
char    newname[100];
int     i;
int     rm_nbr;
struct userinfo *u;
int     unbr;
unsigned char filebuf[8192];


  for (rm_nbr = 0; rm_nbr < MAXROOMS; rm_nbr++)
  {
    u = start;

    sprintf(name, "%srm%d.NEW", WHODIR, rm_nbr);
    sprintf(newname, "%srm%d", WHODIR, rm_nbr);

    if (!(file = fopen(name, "w")))
      return (-1);
    setvbuf(file, filebuf, _IOFBF, sizeof filebuf);

    if (!(msg->room[rm_nbr].flags & QR_INUSE))
    {
      fclose(file);
      rename(name, newname);
      continue;
    }

    /* make a heading in the whoknows file for this room */
    sprintf(filestr, "\nWho knows \"%s\"\n\n", msg->room[rm_nbr].name);

    if (rm_nbr < 2)
    {
      fprintf(file, "%sEVERYONE knows this forum.\n", filestr);
      fclose(file);
      rename(name, newname);
      continue;
    }

    fprintf(file, "%s", filestr);
    *filestr = 0;

    for (unbr = 0; unbr < *ucount; unbr++)
    {
      if (msg->room[rm_nbr].gen != u->forget[rm_nbr] &&
	  u->generation[rm_nbr] != RODSERLING &&
	  (!(msg->room[rm_nbr].flags & QR_PRIVATE) &&
	   u->forget[rm_nbr] != NEWUSERFORGET ||
	   msg->room[rm_nbr].gen == u->generation[rm_nbr]))
      {
	sprintf(temp, "%s (%ld)", u->name, u->usernum);
	i = strlen(temp);
	strcat(temp, "                             " + i);
	if (!*filestr)
	  strcpy(filestr, temp);
	else
	{
	  strcat(filestr, temp);
	  fprintf(file, "%s\n", filestr);
	  *filestr = 0;
	}
      }
      u++;
    }				/* end users for loop */
    if (*filestr)
      fprintf(file, "%s\n", filestr);
    fclose(file);
    rename(name, newname);
  }				/* end rooms for loop */
  return (0);
}


#ifdef _UPDATEHACK
  struct xconf
  {
    unsigned int :7;
    unsigned int which:1;               /* 0 = disable, 1 = enable   */
    unsigned int usernum:24;            /* Username to enable/disable*/
  };

int
hack_xconf(u)
register struct user *u;
{
  register int i;
  struct xconf x[60];

  if ((sizeof x) != 240)
  {
    my_printf("Sizeof x is %d\n", sizeof x);
    return 0;
  }
  bzero(x, sizeof x);
  for (i = 0; i < 10; i++)
  {
    if (u->xno[i].usernum)
      addxconf(u, u->xno[i].usernum, x, 0);
    if (u->xyes[i].usernum)
      addxconf(u, u->xyes[i].usernum, x, 1);
  }
#if 0
  bzero((void *)u->xno, sizeof u->xno);
  bcopy(x, (struct xconf *)(void *)u->xyes, sizeof x);
#endif
  return 0;
}


int
addxconf(u, num, x, which)
register struct user *u;
register long num;
struct xconf x[60];
register int which;
{
  register char *name, *tmpname;
  register int i, j;

  name = getusername(num, 0);
  if (!name)
    return 0;
  for (i = 0; i < 20 && x[i].usernum; i++)
  {
    tmpname = getusername(x[i].usernum, 0);
    if (!tmpname)
    {
      my_printf("Can't happen here\n");
      return 0;
    }
    if (!strcmp(name, tmpname))
      return 0;
    if (strcmp(name, tmpname) < 0)
      break;
  }
  if (i == 20)
  {
    my_printf("List full, can't happen\n");
    return 0;
  }
  for (j = 19; j > i; j--)
    x[j] = x[j - 1];
  x[i].usernum = num;
  x[i].which = which;
  return 0;
}
#endif
