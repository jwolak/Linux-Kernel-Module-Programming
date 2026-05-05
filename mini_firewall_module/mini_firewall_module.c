#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <net/net_namespace.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Janusz Wolak");
MODULE_DESCRIPTION("Mini firewall with Netfilter hook");

static bool drop_ping = true;
module_param(drop_ping, bool, 0644);
MODULE_PARM_DESC(drop_ping, "Drop ICMP echo requests");

static char *blocked_src_ip = "";
module_param(blocked_src_ip, charp, 0644);
MODULE_PARM_DESC(blocked_src_ip, "Blocked source IPv4, e.g. 192.168.1.50");

static ushort blocked_dport = 0;
module_param(blocked_dport, ushort, 0644);
MODULE_PARM_DESC(blocked_dport, "Blocked destination TCP port");

static __be32 blocked_src_ip_be;
static bool blocked_src_ip_enabled;

static struct nf_hook_ops mini_fw_ops;

static unsigned int mini_fw_hook(void *priv, struct sk_buff *skb,
                                 const struct nf_hook_state *state)
{
    struct iphdr *iph;

    if (!skb)
        return NF_ACCEPT;

    iph = ip_hdr(skb);
    if (!iph)
        return NF_ACCEPT;

    if (blocked_src_ip_enabled && iph->saddr == blocked_src_ip_be) {
        pr_info("mini_fw: drop src_ip=%pI4\n", &iph->saddr);
        return NF_DROP;
    }

    if (drop_ping && iph->protocol == IPPROTO_ICMP) {
        struct icmphdr *icmph;

        if (!pskb_may_pull(skb, ip_hdrlen(skb) + sizeof(struct icmphdr)))
            return NF_ACCEPT;

        icmph = icmp_hdr(skb);
        if (icmph && icmph->type == ICMP_ECHO) {
            pr_info("mini_fw: drop ping src=%pI4 dst=%pI4\n", &iph->saddr, &iph->daddr);
            return NF_DROP;
        }
    }

    if (blocked_dport && iph->protocol == IPPROTO_TCP) {
        struct tcphdr *tcph;

        if (!pskb_may_pull(skb, ip_hdrlen(skb) + sizeof(struct tcphdr)))
            return NF_ACCEPT;

        tcph = tcp_hdr(skb);
        if (tcph && ntohs(tcph->dest) == blocked_dport) {
            pr_info("mini_fw: drop tcp dport=%u src=%pI4 dst=%pI4\n",
                    blocked_dport, &iph->saddr, &iph->daddr);
            return NF_DROP;
        }
    }

    return NF_ACCEPT;
}

static int __init mini_fw_init(void)
{
    int ret;

    if (blocked_src_ip && blocked_src_ip[0]) {
        blocked_src_ip_be = in_aton(blocked_src_ip);
        blocked_src_ip_enabled = true;
    }

    mini_fw_ops.hook = mini_fw_hook;
    mini_fw_ops.pf = NFPROTO_IPV4;
    mini_fw_ops.hooknum = NF_INET_PRE_ROUTING;
    mini_fw_ops.priority = NF_IP_PRI_FIRST;

    ret = nf_register_net_hook(&init_net, &mini_fw_ops);
    if (ret) {
        pr_err("mini_fw: nf_register_net_hook failed: %d\n", ret);
        return ret;
    }

    pr_info("mini_fw: loaded drop_ping=%d blocked_src_ip=%s blocked_dport=%u\n",
            drop_ping, blocked_src_ip, blocked_dport);
    return 0;
}

static void __exit mini_fw_exit(void)
{
    nf_unregister_net_hook(&init_net, &mini_fw_ops);
    pr_info("mini_fw: unloaded\n");
}

module_init(mini_fw_init);
module_exit(mini_fw_exit);