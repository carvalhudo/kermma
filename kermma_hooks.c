/**
 * \brief  This file contains the implementation of hooks
 */
#include <linux/kprobes.h>
#include <linux/string.h>

#include "kermma_hooks.h"
#include "kermma_macros.h"

/**
 * \brief  Post handler implementation for module loading
 * \param  probe Kretprobe instance
 * \param  regs Registers associated with the call
 * \return TODO
 */
static int load_module_post(struct kretprobe_instance *probe, struct pt_regs *regs)
{
    int ret = regs_return_value(regs);

    kermma_debug("%s called!\n", __func__);

    if (!ret) {
        kermma_log("ret value: %d\n", ret);
        // TODO: install memory hooks
    }

    probe->rp->handler = NULL;

    return 0;
}

/**
 * \brief  Pre handler implementation for module loading
 * \param  probe Kretprobe instance
 * \param  regs Registers associated with the call
 * \return TODO
 */
static int load_module_pre(struct kretprobe_instance *probe, struct pt_regs *regs)
{
    struct module *mod = NULL;

    kermma_debug("%s called!\n", __func__);

    mod = (struct module *)regs_get_kernel_argument(regs, 0);
    if (mod && !strcmp("foo", mod->name)) {
        kermma_log("target module: %s loaded\n", mod->name);

        /* the post handler is set only for the target module */
        probe->rp->handler = load_module_post;
    }

    return 0;
}

/**
 * \brief  Post handler implementation for module unloading
 * \param  probe Kretprobe instance
 * \param  regs Registers associated with the call
 * \return TODO
 */
static int unload_module_post(struct kretprobe_instance *probe, struct pt_regs *regs)
{
    kermma_debug("%s called!\n", __func__);

    // TODO: uninstall memory hooks

    probe->rp->handler = NULL;

    return 0;
}

/**
 * \brief  Pre handler implementation for module unloading
 * \param  probe Kretprobe instance
 * \param  regs Registers associated with the call
 * \return TODO
 */
static int unload_module_pre(struct kretprobe_instance *probe, struct pt_regs *regs)
{
    struct module *mod = NULL;

    kermma_debug("%s\n", __func__);

    mod = (struct module *)regs_get_kernel_argument(regs, 0);
    if (!strcmp("foo", mod->name)) {
        kermma_log("target module %s unloaded\n", mod->name);

        /* the post handler is set only for the target module */
        probe->rp->handler = unload_module_post;
    }

    return 0;
}

enum Hooks {
    LOAD_MODULE_HOOK,
    UNLOAD_MODULE_HOOK
};

static struct kretprobe hooks[] = {
    [LOAD_MODULE_HOOK] = {
        .entry_handler = load_module_pre,
        .handler = NULL,
        .kp = {
            .symbol_name = "do_init_module"
        }
    },
    [UNLOAD_MODULE_HOOK] = {
        .entry_handler = unload_module_pre,
        .handler = NULL,
        .kp = {
            .symbol_name = "free_module"
        }
    },
};

void register_module_hooks(const char *module)
{
    kermma_log("registering hooks for module %s\n", module);

    (void)register_kretprobe(&hooks[LOAD_MODULE_HOOK]);
    (void)register_kretprobe(&hooks[UNLOAD_MODULE_HOOK]);
}

void unregister_module_hooks(const char *module)
{
    kermma_log("unregistering hooks for module %s\n", module);

    unregister_kretprobe(&hooks[LOAD_MODULE_HOOK]);
    unregister_kretprobe(&hooks[UNLOAD_MODULE_HOOK]);
}

int has_module_hooks(const char *module)
{
    return 0; // TODO
}
