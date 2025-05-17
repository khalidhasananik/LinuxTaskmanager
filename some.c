#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <utmp.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <sys/times.h>

char *getSystemInformation()
{
    struct utsname uts;
    if (uname(&uts) == -1)
    {
        perror("uname error");
        exit(EXIT_FAILURE);
    }

    char *sys_info = (char *)malloc(1024 * sizeof(char));

    snprintf(sys_info, 1024, "System Information:\n"
                             "  - Operating System: %s\n"
                             "  - Version: %s\n"
                             "  - Release: %s\n"
                             "  - Machine: %s\n"
                             "  - Node Name: %s\n",
             uts.sysname, uts.version, uts.release, uts.machine, uts.nodename);

    return sys_info;
}

char *getUptime()
{
    FILE *fp = fopen("/proc/uptime", "r");
    if (fp == NULL)
    {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    char *uptime = (char *)malloc(1024 * sizeof(char));
    char line[1024];
    fgets(line, sizeof(line), fp);
    fclose(fp);

    char *token = strtok(line, " ");
    int uptime_seconds = atoi(token);
    int uptime_minutes = uptime_seconds / 60;
    int uptime_hours = uptime_minutes / 60;
    int uptime_days = uptime_hours / 24;
    int uptime_years = uptime_days / 365;

    snprintf(uptime, 1024, "Uptime: %d years, %d days, %d hours, %d minutes, %d seconds", uptime_years, uptime_days % 365, uptime_hours % 24, uptime_minutes % 60, uptime_seconds % 60);

    return uptime;
}

char *getProcessInfo()
{
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    char *process_info = (char *)malloc(1024 * sizeof(char));
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strncmp(line, "processes", 9) == 0)
        {
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            int processes = atoi(token);
            snprintf(process_info, 1024, "Process Information:\n  - Total: %d", processes);
            break;
        }
    }

    fclose(fp);

    return process_info;
}

char *getNetworkInfo()
{
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    char *network_info = (char *)malloc(1024 * sizeof(char));
    char line[1024];
    unsigned long total_rx_bytes = 0, total_tx_bytes = 0;
    unsigned long total_rx_packets = 0, total_tx_packets = 0;

    // Skip the first two header lines
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        char iface[64];
        unsigned long rx_bytes, rx_packets, rx_errs, rx_drop, rx_fifo, rx_frame, rx_compressed, rx_multicast;
        unsigned long tx_bytes, tx_packets, tx_errs, tx_drop, tx_fifo, tx_colls, tx_carrier, tx_compressed;

        // Parse the line for interface and stats
        int matched = sscanf(line,
                             " %63[^:]: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                             iface,
                             &rx_bytes, &rx_packets, &rx_errs, &rx_drop, &rx_fifo, &rx_frame, &rx_compressed, &rx_multicast,
                             &tx_bytes, &tx_packets, &tx_errs, &tx_drop, &tx_fifo, &tx_colls, &tx_carrier, &tx_compressed);

        if (matched >= 10 && strcmp(iface, "lo") != 0)
        {
            total_rx_bytes += rx_bytes;
            total_rx_packets += rx_packets;
            total_tx_bytes += tx_bytes;
            total_tx_packets += tx_packets;
        }
    }

    snprintf(network_info, 1024, "Network Information:\n  - Packets Sent: %lu\n  - Bytes Sent: %lu\n  - Packets Received: %lu\n  - Bytes Received: %lu",
             total_tx_packets, total_tx_bytes, total_rx_packets, total_rx_bytes);

    fclose(fp);

    return network_info;
}

char *getCPUInformation()
{
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL)
    {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    char *cpu_info = (char *)malloc(1024 * sizeof(char));
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strncmp(line, "model name", 10) == 0)
        {
            char *token = strtok(line, ":");
            token = strtok(NULL, ":");
            snprintf(cpu_info, 1024, "CPU Information:\n\n  - Model Name: %s", token);
            break;
        }
    }

    fclose(fp);

    return cpu_info;
}

char *getMemoryInformation()
{
    struct sysinfo info;
    if (sysinfo(&info) == -1)
    {
        perror("sysinfo error");
        exit(EXIT_FAILURE);
    }

    long total_ram = info.totalram * info.mem_unit / (1024 * 1024);
    long free_ram = info.freeram * info.mem_unit / (1024 * 1024);
    long used_ram = total_ram - free_ram;

    char *mem_info = (char *)malloc(1024 * sizeof(char));
    snprintf(mem_info, 1024, "Memory Information:\n  - Total: %ld MB\n  - Used: %ld MB\n  - Free: %ld MB", total_ram, used_ram, free_ram);

    return mem_info;
}

char *getCPUUsage()
{
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    char *cpu_usage = (char *)malloc(1024 * sizeof(char));
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        if (strncmp(line, "cpu ", 4) == 0)
        {
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            int user = atoi(token);
            token = strtok(NULL, " ");
            int nice = atoi(token);
            token = strtok(NULL, " ");
            int system = atoi(token);
            token = strtok(NULL, " ");
            int idle = atoi(token);

            int total_time = user + nice + system + idle;

            float cpu_usage_t = (1 - (idle * 1.0 / total_time)) * 100;
            snprintf(cpu_usage, 1024, "- CPU Usage: %.2f%%", cpu_usage_t);
            break;
        }
    }

    fclose(fp);

    return cpu_usage;
}

int main()
{
    initscr();
    cbreak();
    noecho();
    clear();
    start_color();

    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_WHITE, COLOR_CYAN);

    WINDOW *sys_win = newwin(9, 84, 1, 2);
    WINDOW *cpu_win = newwin(6, 84, 12, 2);
    WINDOW *mem_win = newwin(6, 84, 19, 2);
    WINDOW *time_win = newwin(3, 84, 26, 2);
    WINDOW *process_win = newwin(4, 84, 30, 2);
    WINDOW *network_win = newwin(7, 84, 35, 2);

    box(sys_win, 0, 0);
    wbkgd(sys_win, COLOR_PAIR(1));
    box(cpu_win, 0, 0);
    wbkgd(cpu_win, COLOR_PAIR(2));
    box(mem_win, 0, 0);
    wbkgd(mem_win, COLOR_PAIR(1));
    box(time_win, 0, 0);
    wbkgd(time_win, COLOR_PAIR(2));
    box(process_win, 0, 0);
    wbkgd(process_win, COLOR_PAIR(1));
    box(network_win, 0, 0);
    wbkgd(network_win, COLOR_PAIR(2));

    while (true)
    {
        werase(sys_win);
        werase(cpu_win);
        werase(mem_win);
        werase(time_win);
        werase(process_win);
        werase(network_win);

        char *sys_info = getSystemInformation();
        char *cpu_info = getCPUInformation();
        char *mem_info = getMemoryInformation();
        char *cpu_usage = getCPUUsage();
        char *uptime = getUptime();
        char *process_info = getProcessInfo();
        char *network_info = getNetworkInfo();

        mvwprintw(sys_win, 1, 2, sys_info);
        mvwprintw(cpu_win, 1, 2, cpu_info);
        mvwprintw(mem_win, 1, 2, mem_info);
        mvwprintw(cpu_win, 2, 2, cpu_usage);
        mvwprintw(time_win, 1, 2, uptime);
        mvwprintw(process_win, 1, 2, process_info);
        mvwprintw(network_win, 1, 2, network_info);

        wrefresh(sys_win);
        wrefresh(cpu_win);
        wrefresh(mem_win);
        wrefresh(time_win);
        wrefresh(process_win);
        wrefresh(network_win);

        free(sys_info);
        free(cpu_info);
        free(mem_info);
        free(cpu_usage);
        free(uptime);
        free(process_info);
        free(network_info);

        sleep(1);
    }

    endwin();

    return 0;
}
