#include "arg_parse.h"

Signal addOptArg_arg_paerser(ARG_PARSER *this, const char* name, const char* full_name, const char *description, Bool required, int least_params)
{
    ARG_OPTION option =
    {
        .name = name,
        .full_name = full_name,
        .description = description,
        .required = required,
        .least_params = least_params,
        .idx = -1,
        .params = 0,
    };
    return this->args_list->append(this->args_list, &option);
}

Signal parseArgs_arg_parser(ARG_PARSER *this, int argc, const char **argv)
{
    ARG_OPTION *last_opt = NULL, *curr_opt = NULL;
    this->argc = argc;
    this->argv = argv;

    if (argc == 1)
    {
        this->showHelpMsg(this);
        return Error;
    }    

    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-')      // is opt
        {
            Bool found = false;
            for (int j = 0; j < this->args_list->length; j++)
            {
                curr_opt = (ARG_OPTION*)(this->args_list->get(this->args_list, j));
                if (
                    strcmp(argv[i], curr_opt->name) == 0 || 
                    strcmp(argv[i], curr_opt->full_name) == 0)
                {
                    last_opt = curr_opt;
                    last_opt->idx = i;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                printf("Unexpected argument: \'%s\'\n", argv[i]);
                return Error;
            }
        }
        else                        // is param
        {
            if (curr_opt != NULL) (curr_opt->params)++;
            else if (i == 0);
            else
            {
                printf("Unexpected parameter: \'%s\'\n", argv[i]);
                return Error;
            }
        }
    }

    for (int i = 0; i < this->args_list->length; i++)
    {
        curr_opt = (ARG_OPTION*)(this->args_list->get(this->args_list, i));
        if (strcmp(curr_opt->name, "-h") == 0 && curr_opt->idx != -1)               // show help info & quit
        {
            this->showHelpMsg(this);
            return Error;
        }
        if (curr_opt->required && curr_opt->idx == -1)                              // missing required argument
        {
            printf("Required argument \'%s\' not found!\n", curr_opt->full_name);
            return Error;
        }
        if (curr_opt->idx != -1 && curr_opt->least_params > curr_opt->params)       // missing parameters
        {
            printf("Need %d params after \'%s\'/\'%s\', got %d params!\n", curr_opt->least_params, curr_opt->name, curr_opt->full_name, curr_opt->params);
            return Error;
        }
    }
    
    return Success;
}

Signal showHelpMsg_arg_parser(ARG_PARSER *this)
{
    ARG_OPTION *curr_opt;
    Vector *examples = Vector_GetInstance(sizeof(char*));
    String *example_str = String_GetInstance("");
    examples->deepFree = true;

    const char *exe_name = this->argv[0] + strlen(this->argv[0]) - 1;
    size_t max_example = 0;
    for (; *exe_name != '/' && *exe_name != '\\'; exe_name--);
    
    printf("usage: %s", exe_name + 1);
    for (int i = 0; i < this->args_list->length; i++)
    {
        curr_opt = (ARG_OPTION*)(this->args_list->get(this->args_list, i));
        char *upper_case_name, *example_cstr;
        cstringUpperCase(&upper_case_name, curr_opt->full_name);

        printf(" ");
        if (!curr_opt->required) printf("[");
        printf("%s", curr_opt->name);
        for (int j = 0; j < curr_opt->least_params; j++) printf(" %s", upper_case_name + 2);
        if (!curr_opt->required) printf("]");

        example_str->clear(example_str);
        example_str->append(example_str, curr_opt->name);
        for (int j = 0; j < curr_opt->least_params; j++) example_str->append(example_str, " %s", upper_case_name + 2);
        example_str->append(example_str, ", %s", curr_opt->full_name);
        for (int j = 0; j < curr_opt->least_params; j++) example_str->append(example_str, " %s", upper_case_name + 2);
        example_cstr = example_str->chrCopy(example_str);
        examples->append(examples, &example_cstr);
        max_example = max_example > strlen(example_cstr) ? max_example : strlen(example_cstr);

        free(upper_case_name);
    }
    printf("\n\n%s\n\noptions:\n", this->description);
    for (int i = 0; i < this->args_list->length; i++)
    {
        curr_opt = (ARG_OPTION*)(this->args_list->get(this->args_list, i));
        printf("  %-*s%s\n", max_example + 2, *((char**)(examples->get(examples, i))), curr_opt->description);
    }

    example_str->delete(example_str);
    examples->delete(examples);
}

ARG_OPTION *findOption_arg_parser(ARG_PARSER *this, const char* name)
{
    ARG_OPTION *curr_opt;
    for (int i = 0; i < this->args_list->length; i++)
    {
        curr_opt = (ARG_OPTION*)(this->args_list->get(this->args_list, i));
        if (
            strcmp(name, curr_opt->name) == 0 || 
            strcmp(name, curr_opt->full_name) == 0)
        {
            return curr_opt;
        }
    }
    return NULL;
}

void delete_arg_parser(ARG_PARSER *this)
{
    this->args_list->delete(this->args_list);
}

ARG_PARSER * ARG_PARSER_GetInstance(const char *description)
{
    ARG_PARSER *instance = malloc(sizeof(ARG_PARSER));
    instance->description = description;
    instance->args_list = Vector_GetInstance(sizeof(ARG_OPTION));
    instance->addArgOpt = addOptArg_arg_paerser;
    instance->parseArgs = parseArgs_arg_parser;
    instance->showHelpMsg = showHelpMsg_arg_parser;
    instance->findOption = findOption_arg_parser;
    instance->delete = delete_arg_parser;

    instance->addArgOpt(instance, "-h", "--help", "show this help message and exit", false, 0);
    return instance;
}

int findOption(int argc, const char **argv, ...)
{
    va_list args;

    for (int i = 0; i < argc; i++)
    {
        const char *arg;
        va_start(args, argv);

        while (*(arg = va_arg(args, const char*)) != '\0')
        {
            if (strcmp(arg, argv[i]) == 0)
            {
                va_end(args);
                return i;
            }
        }
        
        va_end(args);
    }
    
    return -1;
}

int findOptionParams(int argc, const char **argv, int *params, ...)
{
    va_list args;
    int idx_option = -1;

    *params = 0;

    for (int i = 0; i < argc; i++)
    {
        if (idx_option == -1)
        {
            const char *arg;
            va_start(args, params);

            while (*(arg = va_arg(args, const char*)) != '\0')
            {
                if (strcmp(arg, argv[i]) == 0)
                {
                    va_end(args);
                    idx_option = i;
                    break;
                }
            }
            
            va_end(args);
        }
        else
        {
            if (argv[i][0] != '-') (*params)++;
        }
    }
    
    return idx_option;
}
