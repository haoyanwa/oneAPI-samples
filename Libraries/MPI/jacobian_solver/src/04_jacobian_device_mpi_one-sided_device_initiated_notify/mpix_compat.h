/*==============================================================
 * Copyright © 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 * ============================================================= */
#ifndef MPIX_COMPAT_H
#define MPIX_COMPAT_H

#define MPI_ERR_INVALID_NOTIFICATION MPI_ERR_OTHER

/* int MPI_Win_notify_attach(MPI_Win win, int notification_num, MPI_Info info); */
static inline int MPI_Win_notify_set_num(MPI_Win win, MPI_Info info, int notification_num)
{
    if (notification_num == 0) {
        return MPIX_Win_free_notify(win);
    }
    return MPIX_Win_create_notify(win, notification_num);
}

/* int MPI_Win_notify_get_value(MPI_Win win, int notification_idx, MPI_Count *value) */
#define MPI_Win_notify_get_value(win, notification_idx, value) \
            MPIX_Win_get_notify(win, notification_idx, value)

/* int MPI_Win_notify_set_value(MPI_Win win, int notification_idx, MPI_Count value) */
#define MPI_Win_notify_set_value(win, notification_idx, value) \
            MPIX_Win_set_notify(win, notification_idx, value)

#define MPI_Put_notify MPIX_Put_notify
#define MPI_Get_notify MPIX_Get_notify

#endif /* MPIX_COMPAT_H */
