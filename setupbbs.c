#include "defs.h"
#include "ext.h"

void main(int argc, char **argv)
{
  setupbtmp();
  setupmsgmain();
  setupmsgdata();
  setupuserdata();
  setupxmsgdata();
  setupvoteinfo();
}

void setupbtmp()
{
  int fd;
  struct bigbtmp bigbtmp;

  if ((fd = open (TMPDATA, O_RDWR | O_CREAT, 0660)) < 0)
  {
    perror ("can't open tmpdata");
    return;
  }
  
  bzero (&bigbtmp, sizeof (struct bigbtmp));
  write (fd, (char *)&bigbtmp, sizeof (struct bigbtmp));
  close (fd);
  printf ("btmp setup complete\n");
}

void setupmsgmain()
{
  FILE *fp;
  long i;

  if ((fp = fopen (MSGMAIN, "w+")) == NULL)
  {
    perror ("can't open msgmain");
    return;
  }

  for (i = 0; i < MM_FILELEN; i++)
    putc (0, fp);
  fclose (fp);
  printf ("msgmain done\n");
}


void setupmsgdata()
{
  int fd;
  struct msg msg;

  if ((fd = open (MSGDATA, O_RDWR | O_CREAT, 0660)) < 0) {
    perror ("can't open msgdata");
    return;
  }
  
  bzero ((char *) &msg, sizeof (struct msg));
  msg.xmsgsize = 5000000;
  write (fd, (char *)&msg, sizeof (struct msg));
  close (fd);
  printf ("msg data done\n");
}


void setupuserdata()
{
  FILE *fp;
  int i;

  if ((fp = fopen (USERDATA, "w+")) == NULL)
  {
    perror ("can't open userdata");
    return;
  }

  for (i = 0; i < sizeof (struct userdata) + sizeof (struct user) * MAXTOTALUSERS; i++)
    putc (0, fp);
  fclose (fp);
  printf ("userdata done\n");
}


void setupxmsgdata()
{
  FILE *fp;
  int i;

  if ((fp = fopen (XMSGDATA, "w+")) == NULL)
  {
    perror ("can'topen xmsgdata");
    return;
  }
  
  for (i = 0; i < 5000000; i++)
    putc (0, fp);
  fclose (fp);
  printf ("xmsgdata done\n");
}

void setupvoteinfo()
{
  struct voteinfo voteinfo;
  int fd;

  bzero (&voteinfo, sizeof (struct voteinfo));
  if ((fd = open (VOTEFILE, O_RDWR | O_CREAT, 0664)) < 0)
  {
    perror ("voteinfo");
    return;
  }

  write (fd, (char*)& voteinfo, sizeof (struct voteinfo));
  close (fd);
  printf ("Voteinfo done\n");
}
