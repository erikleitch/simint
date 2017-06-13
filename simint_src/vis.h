#ifndef vis_h
#define vis_h

/*
 * Create a data type to encapsulate lists of visiblity indices 
 * associated with each antenna pair.
 */
typedef struct VisList {
  AntType *type1; /* The first antenna of this pair. */
  AntType *type2; /* The second antenna of this pair */
  int nvis;       /* The number of visibilities of this type. */
  int *indices;   /* The indices of these visibilities in the array of n*(n-1)/2 */
  Ant **ant1;     /* A pointer to the first antenna of this baseline */
  Ant **ant2;     /* A pointer to the second antenna of this baseline */
  struct VisList *next; /* The next distinct antenna pairing present */
} VisList;

VisList *construct_list(Simint *sim);
int new_write_vis(FILE *fp, Simint *sim, Dtype type);
int new_write_vis2(FILE *fp, Simint *sim, Dtype type);

#endif
