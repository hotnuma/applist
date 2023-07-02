#if 0

static gint _appinfo_cmp(gconstpointer a, gconstpointer b)
{
    gchar *casefold_a = g_utf8_casefold(g_app_info_get_name(G_APP_INFO(a)), -1);
    gchar *casefold_b = g_utf8_casefold(g_app_info_get_name(G_APP_INFO(b)), -1);

    gint result = g_utf8_collate(casefold_a, casefold_b);

    g_free (casefold_a);
    g_free (casefold_b);

    return result;
}

#endif


