// vim: noet:sw=8:ts=8:tw=80:nowrap
#ifndef PRIVS_H
#define PRIVS_H

/** Drop root privileges.
 * @returns 0 on success and -1 on failure.
 */
int drop_root_privileges(void);

#endif // PRIVS_H
