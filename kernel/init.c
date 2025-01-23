extern void device_init(void);

void w_start(void)
{
    device_init();

    extern int main(void);

	(void)main();
}