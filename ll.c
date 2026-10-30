#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct node
{
	int			data;
	int			index;
	struct node	*next;
}				node;

enum			operators
{
	SA,
	SB,
	SS,
	PA,
	PB,
	RA,
	RB,
	RR,
	RRA,
	RRB,
	RRR
};
enum			e_mode
{
	SIMPLE,
	MEDIUM,
	COMPLEX,
	ADAPTIVE
};
typedef struct s_data
{
	node		*a;
	node		*b;
	int			counter[11];
	int			mode;
	int			bench;
	int			mistakes;
	int			total_pairs;
	int			chosen;
}				t_data;

/*======================== utils =============================*/

/* Prints an int to the given file descriptor (handles negatives and INT_MIN). */

void	ft_putnbr_fd(int n, int fd)
{
	char	digit;

	if (n == -2147483648)
	{
		write(fd, "-2147483648", 11);
		return ;
	}
	if (n < 0)
	{
		write(fd, "-", 1);
		n = -n;
	}
	if (n >= 10)
		ft_putnbr_fd(n / 10, fd);
	digit = (n % 10) + '0';
	write(fd, &digit, 1);
}

/* Prints a string to the given file descriptor (1 = stdout, 2 = stderr). */

void	ft_putstr_fd(char *s, int fd)
{
	int	index;

	if (!s)
		return ;
	index = 0;
	while (s[index])
	{
		write(fd, &s[index], 1);
		index++;
	}
}

/* Prints one character to the given file descriptor. */

void	ft_putchar_fd(char c, int fd)
{
	write(fd, &c, 1);
}

/* Returns 1 if c is a digit character '0' to '9', otherwise 0. */

int	ft_isdigit(int c)
{
	if (c >= 48 && c <= 57)
		return (1);
	return (0);
}

/* Compares two strings, returns 0 when they are equal. */

int	ft_strcmp(const char *s1, const char *s2)
{
	int	i;

	i = 0;
	while (s1[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/* Counts how many words a string has when split by the separator c. */

static int	ft_count_words(char const *s, char c)
{
	int	count;
	int	i;

	count = 0;
	i = 0;
	while (s[i])
	{
		if (s[i] != c)
		{
			count++;
			while (s[i] && s[i] != c)
				i++;
		}
		else
			i++;
	}
	return (count);
}

/* Mallocs and returns a copy of len characters of s starting at start. */

static char	*ft_get_word(char const *s, int start, int len)
{
	char	*word;
	int		i;

	word = (char *)malloc(sizeof(char) * (len + 1));
	if (!word)
		return (NULL);
	i = 0;
	while (i < len)
	{
		word[i] = s[start + i];
		i++;
	}
	word[len] = '\0';
	return (word);
}

/* Fills the result array with the words of s, ending it with NULL. */

static void	ft_fill_words(char const *s, char c, char **result, int count)
{
	int	i;
	int	j;
	int	len;

	i = 0;
	j = 0;
	while (s[i] && j < count)
	{
		if (s[i] != c)
		{
			len = 0;
			while (s[i + len] && s[i + len] != c)
				len++;
			result[j++] = ft_get_word(s, i, len);
			i += len;
		}
		else
			i++;
	}
	result[j] = NULL;
}

/* Splits a string into a NULL-terminated array of words, e.g. "3 1 2". */

char	**ft_split(char const *s, char c)
{
	char	**result;
	int		count;

	if (!s)
		return (NULL);
	count = ft_count_words(s, c);
	result = (char **)malloc(sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	ft_fill_words(s, c, result, count);
	return (result);
}

/* Frees every word of a split array, then the array itself. */

void	freeme_split(char **vals)
{
	int	i;

	i = 0;
	if (!vals)
		return ;
	while (vals[i])
		free(vals[i++]);
	free(vals);
}

/*================== data struct helpers =====================*/

/* Sets both stacks to empty, all counters to 0, and default settings. */

void	data_initialize(t_data *d)
{
	int	index;

	d->a = NULL;
	d->b = NULL;
	index = 0;
	while (index < 11)
	{
		d->counter[index] = 0;
		index++;
	}
	d->mode = ADAPTIVE;
	d->bench = 0;
	d->mistakes = 0;
	d->total_pairs = 0;
	d->chosen = ADAPTIVE;
}

/* Counts one use of an operation and prints its name to stdout. */

void	operator_record(t_data *d, int operation, char *name)
{
	d->counter[operation]++;
	ft_putstr_fd(name, 1);
}

/*===================== linked list ==========================*/

/* Mallocs one node holding val, with index 0 and next set to NULL. */

node	*nodeMaker(int val)
{
	node	*n;

	n = malloc(sizeof(node));
	if (n == NULL)
		return (NULL);
	n->data = val;
	n->index = 0;
	n->next = NULL;
	return (n);
}

/* Adds a new node with val at the end of the list, returns the head. */

node	*insert_last(node *head, int val)
{
	node	*n;
	node	*current;

	n = nodeMaker(val);
	if (n == NULL)
		return (head);
	if (head == NULL)
		return (n);
	current = head;
	while (current->next != NULL)
	{
		current = current->next;
	}
	current->next = n;
	return (head);
}

/* Frees every node of a list and returns NULL. */

node	*free_me(node *top)
{
	node	*current;
	node	*next;

	current = top;
	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	return (NULL);
}

/*====================== the four moves =======================*/

/* Swaps the first two nodes of a stack (does nothing if fewer than 2). */

void	swapper(node **top)
{
	node	*first;
	node	*second;

	if (*top == NULL || (*top)->next == NULL)
		return ;
	first = *top;
	second = first->next;
	first->next = second->next;
	second->next = first;
	*top = second;
}

/* Moves the top node of stack a onto the top of stack b. */

void	trader(node **a, node **b)
{
	node	*n;

	if (*a == NULL)
		return ;
	n = *a;
	*a = n->next;
	n->next = *b;
	*b = n;
}

/* Moves the first node of a stack to the bottom. */

void	rotator(node **top)
{
	node	*first;
	node	*last;

	if (*top == NULL || (*top)->next == NULL)
		return ;
	first = *top;
	last = first;
	while (last->next != NULL)
	{
		last = last->next;
	}
	*top = first->next;
	first->next = NULL;
	last->next = first;
}

/* Moves the last node of a stack to the top. */

void	rev_rotator(node **top)
{
	node	*p;
	node	*l;

	if (*top == NULL || (*top)->next == NULL)
		return ;
	p = NULL;
	l = *top;
	while (l->next != NULL)
	{
		p = l;
		l = l->next;
	}
	p->next = NULL;
	l->next = *top;
	*top = l;
}

/*================== push swap operations =====================*/

/* sa: swap the top two of stack a. */

void	sa(t_data *d)
{
	swapper(&d->a);
	operator_record(d, SA, "sa\n");
}

/* sb: swap the top two of stack b. */

void	sb(t_data *d)
{
	swapper(&d->b);
	operator_record(d, SB, "sb\n");
}

/* ss: sa and sb at the same time (counted as one operation). */

void	ss(t_data *d)
{
	swapper(&d->a);
	swapper(&d->b);
	operator_record(d, SS, "ss\n");
}

/* pa: move the top of b onto a. */

void	pa(t_data *d)
{
	trader(&d->b, &d->a);
	operator_record(d, PA, "pa\n");
}

/* pb: move the top of a onto b. */

void	pb(t_data *d)
{
	trader(&d->a, &d->b);
	operator_record(d, PB, "pb\n");
}

/* ra: rotate a (top goes to the bottom). */

void	ra(t_data *d)
{
	rotator(&d->a);
	operator_record(d, RA, "ra\n");
}

/* rb: rotate b (top goes to the bottom). */

void	rb(t_data *d)
{
	rotator(&d->b);
	operator_record(d, RB, "rb\n");
}

/* rr: ra and rb at the same time (counted as one operation). */

void	rr(t_data *d)
{
	rotator(&d->a);
	rotator(&d->b);
	operator_record(d, RR, "rr\n");
}

/* rra: reverse rotate a (bottom goes to the top). */

void	rra(t_data *d)
{
	rev_rotator(&d->a);
	operator_record(d, RRA, "rra\n");
}

/* rrb: reverse rotate b (bottom goes to the top). */

void	rrb(t_data *d)
{
	rev_rotator(&d->b);
	operator_record(d, RRB, "rrb\n");
}

/* rrr: rra and rrb at the same time (counted as one operation). */

void	rrr(t_data *d)
{
	rev_rotator(&d->a);
	rev_rotator(&d->b);
	operator_record(d, RRR, "rrr\n");
}

/*================== parsing the input ========================*/

/* Frees both stacks, prints "Error" on stderr and exits. */

void	ft_error(node **a, node **b)
{
	if (a && *a)
		free_me(*a);
	if (b && *b)
		free_me(*b);
	ft_putstr_fd("Error\n", 2);
	exit(1);
}

/* Returns 1 if the stack is sorted in ascending order, otherwise 0. */

int	is_sorted(node *stack)
{
	while (stack && stack->next)
	{
		if (stack->data > stack->next->data)
			return (0);
		stack = stack->next;
	}
	return (1);
}

/* Converts a string to an int; returns 0 if invalid or out of int range. */

int	atoi_on_drugs(const char *str, int *out)
{
	int		i;
	long	sign;
	long	result;

	if (!str || !out)
		return (0);
	i = 0;
	sign = 1;
	result = 0;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (!ft_isdigit((unsigned char)str[i]))
			return (0);
		result = result * 10 + (str[i] - '0');
		if (sign * result > INT_MAX || sign * result < INT_MIN)
			return (0);
		i++;
	}
	*out = (int)(sign * result);
	return (1);
}

/* Returns 1 if value already exists in the stack, otherwise 0. */

int	has_dup(node *stack, int value)
{
	while (stack)
	{
		if (stack->data == value)
			return (1);
		stack = stack->next;
	}
	return (0);
}

/* Reads the numbers from av (starting at start) into stack a, or errors. */

void	bob_the_builder(int ac, char **av, t_data *d, int start)
{
	char	**args;
	int		val;
	int		i;
	int		j;

	i = start;
	while (i < ac)
	{
		args = ft_split(av[i], ' ');
		if (!args || !args[0])
		{
			freeme_split(args);
			ft_error(&d->a, &d->b);
		}
		j = 0;
		while (args[j])
		{
			if (!atoi_on_drugs(args[j], &val) || has_dup(d->a, val))
			{
				freeme_split(args);
				ft_error(&d->a, &d->b);
			}
			d->a = insert_last(d->a, val);
			j++;
		}
		freeme_split(args);
		i++;
	}
}

/* Reads the --flags at the start of av, returns where the numbers begin. */

int	flagger(int ac, char **av, t_data *d)
{
	int	i;

	i = 1;
	while (i < ac && av[i][0] == '-' && av[i][1] == '-')
	{
		if (!ft_strcmp(av[i], "--simple"))
			d->mode = SIMPLE;
		else if (!ft_strcmp(av[i], "--medium"))
			d->mode = MEDIUM;
		else if (!ft_strcmp(av[i], "--complex"))
			d->mode = COMPLEX;
		else if (!ft_strcmp(av[i], "--adaptive"))
			d->mode = ADAPTIVE;
		else if (!ft_strcmp(av[i], "--bench"))
			d->bench = 1;
		else
			ft_error(&d->a, &d->b);
		i++;
	}
	return (i);
}

/*================ disorder and adaptive choice ===============*/

/* Returns how many nodes the stack has. */

int	stack_size(node *top)
{
	int	count;

	count = 0;
	while (top)
	{
		count++;
		top = top->next;
	}
	return (count);
}

/* Counts the pairs where an earlier number is bigger than a later one. */

int	count_mistakes(node *a)
{
	node	*i;
	node	*j;
	int		mistakes;

	mistakes = 0;
	i = a;
	while (i)
	{
		j = i->next;
		while (j)
		{
			if (i->data > j->data)
				mistakes++;
			j = j->next;
		}
		i = i->next;
	}
	return (mistakes);
}

/* Stores mistakes and total pairs of the starting stack (call before moves). */

void	compute_disorder(t_data *d)
{
	int	n;

	n = stack_size(d->a);
	d->mistakes = count_mistakes(d->a);
	d->total_pairs = n * (n - 1) / 2;
}

/* Adaptive choice: disorder <0.2 simple, <0.5 medium, otherwise complex. */

int	pick_strategy(t_data *d)
{
	if (d->total_pairs == 0)
		return (SIMPLE);
	if (d->mistakes * 10 < d->total_pairs * 2)
		return (SIMPLE);
	if (d->mistakes * 10 < d->total_pairs * 5)
		return (MEDIUM);
	return (COMPLEX);
}

/*===================== --bench report ========================*/

/* Prints the disorder as a percentage with two decimals, on stderr. */

void	print_disorder(t_data *d)
{
	long	scaled;

	scaled = 0;
	if (d->total_pairs > 0)
		scaled = (long)d->mistakes * 10000 / d->total_pairs;
	ft_putstr_fd("[bench] disorder: ", 2);
	ft_putnbr_fd(scaled / 100, 2);
	ft_putchar_fd('.', 2);
	if (scaled % 100 < 10)
		ft_putchar_fd('0', 2);
	ft_putnbr_fd(scaled % 100, 2);
	ft_putstr_fd("%\n", 2);
}

/* Prints the strategy name and its complexity class, on stderr. */

void	print_strategy(t_data *d)
{
	ft_putstr_fd("[bench] strategy: ", 2);
	if (d->mode == SIMPLE)
		ft_putstr_fd("Simple / O(n^2)", 2);
	else if (d->mode == MEDIUM)
		ft_putstr_fd("Medium / O(n√n)", 2);
	else if (d->mode == COMPLEX)
		ft_putstr_fd("Complex / O(n log n)", 2);
	else
	{
		ft_putstr_fd("Adaptive / ", 2);
		if (d->chosen == SIMPLE)
			ft_putstr_fd("O(n^2)", 2);
		else if (d->chosen == MEDIUM)
			ft_putstr_fd("O(n√n)", 2);
		else
			ft_putstr_fd("O(n log n)", 2);
	}
	ft_putchar_fd('\n', 2);
}

/* Returns the total number of operations used so far. */

int	total_ops(t_data *d)
{
	int	i;
	int	total;

	i = 0;
	total = 0;
	while (i < 11)
	{
		total += d->counter[i];
		i++;
	}
	return (total);
}

/* Prints one "name: value" pair on stderr. */

void	print_pair(char *name, int value)
{
	ft_putstr_fd(name, 2);
	ft_putstr_fd(": ", 2);
	ft_putnbr_fd(value, 2);
}

/* Prints the total and the count of each of the 11 operations, on stderr. */

void	print_counts(t_data *d)
{
	ft_putstr_fd("[bench] total_ops: ", 2);
	ft_putnbr_fd(total_ops(d), 2);
	ft_putstr_fd("\n[bench] ", 2);
	print_pair("sa", d->counter[SA]);
	print_pair(" sb", d->counter[SB]);
	print_pair(" ss", d->counter[SS]);
	print_pair(" pa", d->counter[PA]);
	print_pair(" pb", d->counter[PB]);
	ft_putstr_fd("\n[bench] ", 2);
	print_pair("ra", d->counter[RA]);
	print_pair(" rb", d->counter[RB]);
	print_pair(" rr", d->counter[RR]);
	print_pair(" rra", d->counter[RRA]);
	print_pair(" rrb", d->counter[RRB]);
	print_pair(" rrr", d->counter[RRR]);
	ft_putchar_fd('\n', 2);
}

/* Prints the whole bench report, only if --bench was given. */

void	print_bench(t_data *d)
{
	if (!d->bench)
		return ;
	print_disorder(d);
	print_strategy(d);
	print_counts(d);
}

/*================ simple: O(n^2) selection ===================*/

/* Returns the position (0 = top) of the smallest value in a non-empty stack. */

int	minimum_finder(node *top)
{
	int	min;
	int	idx;
	int	i;

	min = top->data;
	idx = 0;
	i = 0;
	while (top)
	{
		if (top->data < min)
		{
			min = top->data;
			idx = i;
		}
		i++;
		top = top->next;
	}
	return (idx);
}

/* Rotates the smallest of a to the top (shorter way), then pushes it to b. */

void	push_min_to_b(t_data *d)
{
	int	idx;
	int	size;

	idx = minimum_finder(d->a);
	size = stack_size(d->a);
	if (idx <= size / 2)
	{
		while (idx-- > 0)
			ra(d);
	}
	else
	{
		idx = size - idx;
		while (idx-- > 0)
			rra(d);
	}
	pb(d);
}

/* Simple strategy: push the smallest to b until a is empty, then pa all. */

void	sort_simple(t_data *d)
{
	while (d->a)
		push_min_to_b(d);
	while (d->b)
		pa(d);
}

/*================ medium: O(n*sqrt(n)) chunks ================*/

/* Replaces each value by its rank (0 = smallest) in the index field. */

void	assign_index(node *a)
{
	node	*cur;
	node	*other;
	int		rank;

	cur = a;
	while (cur)
	{
		rank = 0;
		other = a;
		while (other)
		{
			if (other->data < cur->data)
				rank++;
			other = other->next;
		}
		cur->index = rank;
		cur = cur->next;
	}
}

/* Returns the integer square root of n (we cannot use sqrt). */

int	int_sqrt(int n)
{
	int	r;

	r = 0;
	while ((r + 1) * (r + 1) <= n)
		r++;
	return (r);
}

/* Phase 1: pushes a to b in chunks of ranks; small ranks go to b's bottom. */

void	push_chunks(t_data *d, int chunk)
{
	int	i;

	i = 0;
	while (d->a)
	{
		if (d->a->index <= i)
		{
			pb(d);
			rb(d);
			i++;
		}
		else if (d->a->index <= i + chunk)
		{
			pb(d);
			i++;
		}
		else
			ra(d);
	}
}

/* Returns the position (0 = top) of the biggest index in a non-empty stack. */

int	max_pos(node *top)
{
	int	max;
	int	pos;
	int	i;

	max = top->index;
	pos = 0;
	i = 0;
	while (top)
	{
		if (top->index > max)
		{
			max = top->index;
			pos = i;
		}
		i++;
		top = top->next;
	}
	return (pos);
}

/* Phase 2: brings the biggest of b to the top (shorter way), pa, repeat. */

void	push_back(t_data *d)
{
	int	pos;
	int	size;

	while (d->b)
	{
		pos = max_pos(d->b);
		size = stack_size(d->b);
		if (pos <= size / 2)
		{
			while (pos-- > 0)
				rb(d);
		}
		else
		{
			pos = size - pos;
			while (pos-- > 0)
				rrb(d);
		}
		pa(d);
	}
}

/* Medium strategy: rank the numbers, push in chunks of ~sqrt(n), push back. */

void	sort_medium(t_data *d)
{
	assign_index(d->a);
	push_chunks(d, int_sqrt(stack_size(d->a)));
	push_back(d);
}

/*================ complex: O(n log n) radix ==================*/

/* Returns how many bits are needed to write the biggest rank (n - 1). */

int	max_bits(int n)
{
	int	bits;

	bits = 0;
	while (((n - 1) >> bits) != 0)
		bits++;
	return (bits);
}

/* One radix pass on one bit: bit 1 stays in a (ra), bit 0 goes to b, then pa. */

void	radix_pass(t_data *d, int bit)
{
	int	size;

	size = stack_size(d->a);
	while (size-- > 0)
	{
		if ((d->a->index >> bit) & 1)
			ra(d);
		else
			pb(d);
	}
	while (d->b)
		pa(d);
}

/* Complex strategy: rank the numbers, then one radix pass per bit. */

void	sort_complex(t_data *d)
{
	int	bit;
	int	bits;

	assign_index(d->a);
	bits = max_bits(stack_size(d->a));
	bit = 0;
	while (bit < bits)
	{
		radix_pass(d, bit);
		bit++;
	}
}

/*======================== run + main =========================*/

/* Picks the strategy (flag or adaptive) and runs it unless already sorted. */

void	run_strategy(t_data *d)
{
	d->chosen = d->mode;
	if (d->mode == ADAPTIVE)
		d->chosen = pick_strategy(d);
	if (d->a == NULL || is_sorted(d->a))
		return ;
	if (d->chosen == SIMPLE)
		sort_simple(d);
	else if (d->chosen == MEDIUM)
		sort_medium(d);
	else
		sort_complex(d);
}

/* Reads flags and numbers, measures disorder, sorts, prints, frees. */

int	main(int ac, char **av)
{
	t_data	d;
	int		start;

	if (ac < 2)
		return (0);
	data_initialize(&d);
	start = flagger(ac, av, &d);
	bob_the_builder(ac, av, &d, start);
	compute_disorder(&d);
	run_strategy(&d);
	print_bench(&d);
	free_me(d.a);
	free_me(d.b);
	return (0);
}