
/*
    Example of use 1:
        #include "Run_task_once.h"

        void my_task(void)
        {
            NRF_LOG_DEBUG("Executing task only once...");
            NRF_LOG_FLUSH();
        }

        Run_task_once important_task(my_task);

        void main()
        {
            while (1)
            {
                important_task.run_task_once();
            }
        }

    Example of use 2:
        If the task you want to execute is a method of an object, you must
        first wrap the method in a free function.

        #include "Run_task_once.h"

        class My_class
        {
          public:
            void my_method_task(void)
            {
                NRF_LOG_DEBUG("Executing method only once...");
                NRF_LOG_FLUSH();
            }
        };

        My_class my_object;

        void my_task(void)
        {
            my_object.my_method_task();
        }

        Run_task_once important_task(my_task);

        void main()
        {
            while (1)
            {
                important_task.run_task_once();  // To rerun this task once, you can call somewhere the its reset() method.
            }
        }
*/

#ifndef _RUN_TASK_ONCE_H_
#define _RUN_TASK_ONCE_H_


// Definimos el tipo FuncType como un puntero a función que retorna void y no toma argumentos.
using func_type_t = void (*)(void);


class Run_task_once
{
  public:
    Run_task_once(func_type_t task) : _task(task) {}

    bool run_task_once(void)
    {
        if (flag)
        {
            flag = false;
            _task();

            return true;
        }

        return false;
    }

    void reset(void)
    {
        flag = true;
    }

  private:
    bool flag = true;
    func_type_t _task;
};


#endif // _RUN_TASK_ONCE_H_
