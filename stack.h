struct stack
{
  command_t *commands;
  int size;
  int max_size;
  void (* push)();
  command_t (* pop)();
  command_t (* peek)();
};
 
void
push (stack *self, command_t command)
{
  if (self->size == self->max_size) {
    self->max_size *= 2;
    command_t *new = malloc (self->max_size * sizeof (command_t));
    self->commands = new;
  }
 
  self->commands[self->size] = command;
  self->size++;
}
 
command_t
pop (stack *self)
{
  if (self->size > 0)
    return self->commands[self->size--];
  else
    return NULL;
}
 
command_t
peek (stack *self)
{
  if (self->size > 0) {
    int i = self->size - 1;
    return self->commands[i];
  } else
    return NULL;
}
 
stack*
init_stack (int max)
{
  stack *new = malloc (sizeof (stack));
  new->size = 0;
  new->max_size = max;
  new->commands = malloc (max * sizeof (command_t));
}